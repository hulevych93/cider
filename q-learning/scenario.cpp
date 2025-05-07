#include "scenario.h"

#include <iostream>
#include <sstream>

namespace cider {
namespace qleaning {

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

QActionList Scenario::getCurrentState() const {
  return m_actions;
}

bool Scenario::isOver() const {
  return m_lastObjVal.coverage > m_initialObjVal.coverage ||
         (abs(m_lastObjVal.coverage - m_initialObjVal.coverage) <
          std::numeric_limits<double>::epsilon()) ||
         (m_size <= m_actions.size());
}

std::optional<QValue> Scenario::getReward() const {
  const auto objValue = m_objFunc(m_actions);
  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
    std::cout << "Candidate: " << objValue.coverage << std::endl;

    if (objValue.coverage > m_lastObjVal.coverage) {
      m_lastObjVal = objValue;
      return 1.0;
    } else if (hasNewCoverageBit(objValue.coveredTracks,
                                 m_lastObjVal.coveredTracks)) {
      m_lastObjVal = objValue;
      return 0.5;
    } else {
      return 0.05;
    }

  } else {
    return std::nullopt;
  }
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
