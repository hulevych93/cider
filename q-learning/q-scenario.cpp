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

inline double normalize_reward(double reward) {
  return reward / (1 + std::abs(reward));
}

std::optional<QValue> rewardFunction(
    RewardCounter& rwCounter,
    const ObjectiveValue& objValue,
    const ObjectiveValue& targetValue,
    double biggerCoverageReward,
    double newTracksReward,
    const std::function<double()>& sameCoverageReward,
    double penalty) {
  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
    const auto coverageBigger = objValue.coverage > targetValue.coverage;
    const auto coverageSame = abs(objValue.coverage - targetValue.coverage) <
                              std::numeric_limits<double>::epsilon();

    std::cout << objValue.coverage << std::endl;

    if (coverageBigger) {
      rwCounter.covGrow++;
      return biggerCoverageReward;
    } else if (coverageSame && hasNewCoverageBit(objValue.coveredTracks,
                                                 targetValue.coveredTracks)) {
      rwCounter.trackGrow++;
      return newTracksReward;
    } else if (coverageSame) {
      return sameCoverageReward();
    } else {
      rwCounter.penalty++;
      return -penalty;
    }

  } else {
    return std::nullopt;
  }
}

double calculateContinuousMultiplier(RewardCounter& rwCounter,
                                     size_t initialSize,
                                     size_t currentSize) {
  if (currentSize < initialSize) {
    rwCounter.scriptLower++;
    double ratio = static_cast<double>(initialSize - currentSize) / initialSize;
    double multiplier = 1.0 + ratio;
    return std::min(2.0, multiplier);
  } else if (currentSize > initialSize) {
    rwCounter.scriptBigger++;
    double ratio = static_cast<double>(currentSize - initialSize) / initialSize;
    double multiplier = 0.5 - 0.3 * ratio;
    return std::max(0.2, multiplier);
  } else {
    rwCounter.scriptSame++;
    return 0.5;
  }
}

}  // namespace

QScenario::QScenario(RewardCounter& counter,
                     std::mt19937& gen,
                     int maxStateDepth,
                     const QActionList& initial,
                     const ObjectiveFunction& objFunc)
    : m_rwCounter(counter),
      m_gen(gen),
      m_maxStateDepth(maxStateDepth),
      m_initialSize(initial.size()),
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

void QScenario::store() {
  std::copy(m_actions.cbegin(), m_actions.cend(),
            std::back_inserter(m_storage));
  m_actions.clear();
  std::cout << "SIZE " << m_actions.size() << std::endl;
}

QActionList QScenario::getResult() const {
  std::copy(m_actions.cbegin(), m_actions.cend(),
            std::back_inserter(m_storage));
  m_actions.clear();
  return m_storage;
}

bool QScenario::isOver() const {
  return isOverFunc(m_lastObjVal, m_initialObjVal, m_availableActions.empty());
}

std::optional<QValue> QScenario::getReward() const {
  auto tempActions = m_storage;
  std::copy(m_actions.cbegin(), m_actions.cend(),
            std::back_inserter(tempActions));
  const auto objValue = m_objFunc(tempActions);

  std::optional<QValue> result;

  if (isOverFunc(objValue, m_initialObjVal, m_availableActions.empty())) {
    result = rewardFunction(
        m_rwCounter, objValue, m_initialObjVal, 5.0, 5.0, []() { return 1.0; },
        5.0);
  } else {
    result = rewardFunction(
        m_rwCounter, objValue, m_lastObjVal, 0.2, 0.1,
        [&]() {
          if (m_actions.size() >= 2U) {
            if (cider::recorder::semanticallyEqual(
                    m_actions[m_actions.size() - 1],
                    m_actions[m_actions.size() - 2])) {
              if (m_actions.size() >= 3U) {
                if (cider::recorder::semanticallyEqual(
                        m_actions[m_actions.size() - 2],
                        m_actions[m_actions.size() - 3])) {
                  m_rwCounter.threeSameAct++;
                  return -0.2;
                }
              }
              m_rwCounter.twoSameAct++;
              return -0.15;
            }
          }

          m_rwCounter.sameCov++;
          return -0.1;
        },
        0.1);
  }

  if (result.has_value()) {
    m_lastObjVal = objValue;

    const double lenghtMultiplier = calculateContinuousMultiplier(
        m_rwCounter, m_initialSize, tempActions.size());

    std::cout << "Multi: " << lenghtMultiplier << ", old: " << m_initialSize
              << ", new: " << tempActions.size() << std::endl;

    result = normalize_reward(result.value() * lenghtMultiplier);
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
