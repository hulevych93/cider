#include "scenario.h"

#include <iostream>
#include <sstream>

namespace cider {
namespace qleaning {

std::string actionToGenericRepro(const QAction& action) {
  std::stringstream os;
  std::visit(
      [&os](auto&& value) {
        using T = std::decay_t<decltype(value)>;

        auto printParams = [&os](const auto& args) {
          os << "(";
          for (size_t i = 0; i < args.size(); ++i) {
            os << "_";  // assuming `typeName` gives a string like "int"
            if (i + 1 != args.size())
              os << ", ";
          }
          os << ")";
        };

        if constexpr (std::is_same_v<T, cider::recorder::Function>) {
          os << value.name;
          printParams(value.params);  // assuming .arguments is a list of types
        } else if constexpr (std::is_same_v<T, cider::recorder::ClassMethod>) {
          os << "obj@->" << value.method.name;
          printParams(value.method.params);  // nested method inside ClassMethod
        } else if constexpr (std::is_same_v<T,
                                            cider::recorder::ClassBinaryOp>) {
          std::string opStr =
              (value.opName == cider::recorder::BinaryOpType::Assignment) ? "="
                                                                          : "?";
          os << "obj@ " << opStr;
        } else if constexpr (std::is_same_v<T, cider::recorder::ClassUnaryOp>) {
          std::string opStr =
              (value.opName == cider::recorder::UnaryOpType::Minus) ? "-" : "?";
          os << opStr << "obj@";
        } else if constexpr (std::is_same_v<T,
                                            cider::recorder::ClassDestructor>) {
          os << "destroy(obj@)";
          // likely no params for destructor
        } else {
          static_assert(!sizeof(T), "Unsupported Action type");
        }
      },
      action);
  return os.str();
}

namespace {

bool hasNewCoverageBit(const std::vector<std::uint8_t>& current,
                       const std::vector<std::uint8_t>& previous) {
  const size_t size = std::min(current.size(), previous.size());
  for (size_t i = 0; i < size; ++i) {
    if ((current[i] & ~previous[i]) != 0) {
      return true;  // New bit discovered
    }
  }
  return false;  // No new bits
}

std::string actionsToString(const QActionList& actions, size_t size) {
  std::ostringstream oss;

  if (actions.empty()) {
    oss << "Empty";
  } else {
    size_t count = 0;
    for (auto it = actions.rbegin(); it != actions.rend() && count < size;
         ++it, ++count) {
      oss << actionToGenericRepro(*it) << ";";
    }
  }

  return oss.str();
}

bool isOverFunc(const ObjectiveValue& objValue,
                const ObjectiveValue& targetValue,
                bool sizeOver) {
  const auto coverageBigger = objValue.coverage > targetValue.coverage;
  const auto coverageSame = abs(objValue.coverage - targetValue.coverage) <
                            std::numeric_limits<double>::epsilon();
  const auto over = coverageBigger || coverageSame || sizeOver;
  if (over) {
    std::cout << "[" << coverageBigger << "," << coverageSame << "," << sizeOver
              << "]" << std::endl;
  }
  return over;
}

inline float normalize_reward(float reward) {
  return reward / (1 + std::abs(reward));
}

}  // namespace

std::optional<QValue> rewardFunction(const ObjectiveValue& objValue,
                                     const ObjectiveValue& targetValue,
                                     double bigReward,
                                     double middleReward,
                                     double lowReward,
                                     double penalty) {
  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
    const auto coverageBigger = objValue.coverage > targetValue.coverage;
    const auto coverageSame = abs(objValue.coverage - targetValue.coverage) <
                              std::numeric_limits<double>::epsilon();

    std::cout << objValue.coverage << std::endl;
    if (coverageBigger) {
      return bigReward;
    } else if (hasNewCoverageBit(objValue.coveredTracks,
                                 targetValue.coveredTracks)) {
      return middleReward;
    } else if (coverageSame) {
      return lowReward;
    } else {
      return penalty;
    }

  } else {
    return std::nullopt;
  }
}

Scenario::Scenario(std::mt19937& gen,
                   int maxStateDepth,
                   const QActionList& initial,
                   const ObjectiveFunction& objFunc)
    : m_gen(gen),
      m_maxStateDepth(maxStateDepth),
      m_size(initial.size()),
      m_objFunc(objFunc) {
  for (const auto& action : initial) {
    m_availableActions.emplace(action);
  }

  const auto objValue = m_objFunc(initial);
  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
    std::cout << "Initial: " << objValue.coverage << std::endl;
    m_initialObjVal = objValue;
  } else {
    throw std::logic_error{"Bad initial script."};
  }
}

void Scenario::add(const QAction& action) {
  m_actions.push_back(action);

  const auto avIt = m_availableActions.find(action);
  if (avIt != m_availableActions.cend()) {
    m_availableActions.erase(avIt);
  }
}

void Scenario::rollback() {
  const auto& action = m_actions.back();
  m_availableActions.emplace(action);

  m_actions.pop_back();
}

void Scenario::rollbackAndDrop() {
  m_actions.pop_back();
}

void Scenario::drop(const QAction& action) {
  const auto avIt = m_availableActions.find(action);
  if (avIt != m_availableActions.cend()) {
    m_availableActions.erase(avIt);
  }
}

QActionList Scenario::getCurrentState() const {
  return m_actions;
}

bool Scenario::isOver() const {
  const auto sizeOver = m_size <= m_actions.size();
  return isOverFunc(m_lastObjVal, m_initialObjVal, sizeOver);
}

std::optional<QValue> Scenario::getReward() const {
  const auto objValue = m_objFunc(m_actions);

  std::optional<QValue> result;

  const auto sizeOver = m_size <= m_actions.size();
  if (isOverFunc(objValue, m_initialObjVal, sizeOver)) {
    std::cout << "final" << std::endl;

    result = rewardFunction(objValue, m_initialObjVal, 4.0, 2.0, 1.0, -4.0);
  } else {
    result = rewardFunction(objValue, m_lastObjVal, 1.0, 0.5, 0.1, -0.5);
  }

  if (result.has_value()) {
    m_lastObjVal = objValue;
    result = result.value();
  }

  return result;
}

std::string Scenario::toString() const {
  return actionsToString(m_actions, m_maxStateDepth);
}

QActionList Scenario::getAvailableActions() const {
  QActionList actions;
  actions.reserve(m_availableActions.size());
  for (const auto& action : m_availableActions) {
    actions.emplace_back(action);
  }
  return actions;
}

std::optional<QAction> Scenario::getRandomAction() const {
  const auto actions = getAvailableActions();
  if (actions.empty()) {
    return std::nullopt;
  }
  std::uniform_int_distribution<size_t> indexDist(0, actions.size() - 1);
  return actions[indexDist(m_gen)];
}

}  // namespace qleaning
}  // namespace cider
