#include "q-scenario.h"

#include <iostream>
#include <sstream>

#include <assert.h>

namespace cider {
namespace qleaning {

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

bool isOverFunc(const ObjectiveValue& objValue,
                const ObjectiveValue& targetValue,
                const bool noActions) {
  const auto coverageBigger = objValue.coverage > targetValue.coverage;
  const auto coverageSame = abs(objValue.coverage - targetValue.coverage) <
                            std::numeric_limits<double>::epsilon();

  const auto over = coverageBigger || coverageSame || noActions;
  if (over) {
    std::cout << "[" << coverageBigger << "," << coverageSame << ","
              << noActions << "]" << std::endl;
  }
  return over;
}

inline float normalize_reward(float reward) {
  return reward / (1 + std::abs(reward));
}

}  // namespace

std::optional<QValue> rewardFunction(
    const ObjectiveValue& objValue,
    const ObjectiveValue& targetValue,
    double biggerCoverageReward,
    double newTracksReward,
    const std::function<double()>& sameCoverageReward,
    double penalty) {
  const auto threshold = 0.01;

  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
    const auto coverageBigger = objValue.coverage > targetValue.coverage;
    const auto coverageSame =
        abs(objValue.coverage - targetValue.coverage) < threshold;

    std::cout << objValue.coverage << std::endl;
    if (coverageBigger) {
      return biggerCoverageReward;
    } else if (hasNewCoverageBit(objValue.coveredTracks,
                                 targetValue.coveredTracks)) {
      return newTracksReward;
    } else if (coverageSame) {
      return sameCoverageReward();
    } else {
      return -penalty;
    }

  } else {
    return std::nullopt;
  }
}

QScenario::QScenario(std::mt19937& gen,
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

void QScenario::add(const QAction& action) {
  m_actions.push_back(action);

  const auto avIt = m_availableActions.find(action);
  if (avIt != m_availableActions.cend()) {
    m_availableActions.erase(avIt);
  }
}

void QScenario::rollback() {
  const auto& action = m_actions.back();
  m_availableActions.emplace(action);

  m_actions.pop_back();
}

QActionList QScenario::getCurrentState() const {
  auto count = m_maxStateDepth;
  if (count > m_actions.size()) {
    count = m_actions.size();
  }

  return QActionList{m_actions.end() - count, m_actions.end()};
}

QActionList QScenario::getResult() const {
  return m_actions;
}

bool QScenario::isOver() const {
  return isOverFunc(m_lastObjVal, m_initialObjVal, m_availableActions.empty());
}

std::optional<QValue> QScenario::getReward() const {
  const auto objValue = m_objFunc(m_actions);

  std::optional<QValue> result;

  if (isOverFunc(objValue, m_initialObjVal, m_availableActions.empty())) {
    result = rewardFunction(
        objValue, m_initialObjVal, 10.0, 5.0, []() { return 3.0; }, 5.0);
  } else {
    result = rewardFunction(
        objValue, m_lastObjVal, 1.0, 0.5,
        [&]() {
          assert(m_actions.size() >= 2U);
          if (cider::recorder::semanticallyEqual(
                  m_actions[m_actions.size() - 1],
                  m_actions[m_actions.size() - 2])) {
            return -0.5;
          }

          return 0.0;
        },
        0.5);
  }

  if (result.has_value()) {
    m_lastObjVal = objValue;
    result = result.value();
  }

  return result;
}

QActionList QScenario::getAvailableActions() const {
  QActionList actions;
  actions.reserve(m_availableActions.size());
  for (const auto& action : m_availableActions) {
    actions.emplace_back(action);
  }
  return actions;
}

std::optional<QAction> QScenario::getRandomAction() const {
  const auto actions = getAvailableActions();
  if (actions.empty()) {
    return std::nullopt;
  }
  std::uniform_int_distribution<size_t> indexDist(0, actions.size() - 1);
  return actions[indexDist(m_gen)];
}

}  // namespace qleaning
}  // namespace cider
