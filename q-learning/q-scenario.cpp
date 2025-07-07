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

inline double normalize_reward(double reward) {
  return reward / (1 + std::abs(reward));
}

std::optional<double> rewardFunction(
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
                     const recorder::Actions& initial,
                     const synthesis::ObjectiveFunction& objFunc)
    : Scenario(gen, maxStateDepth, initial, objFunc), _rwCounter(counter) {}

std::optional<double> QScenario::getReward() const {
  const auto objValue = _objFunc(_actions);

  std::optional<double> result;

  if (synthesis::isOverFunc(objValue, _initialObjVal,
                            _availableActions.empty())) {
    result = rewardFunction(
        _rwCounter, objValue, _initialObjVal, 5.0, 5.0,
        [&]() {
          if (_initialSize > _actions.size()) {
            return 3.0;
          } else {
            return -1.0;
          }
        },
        5.0);
  } else {
    result = rewardFunction(
        _rwCounter, objValue, _lastObjVal, 0.2, 0.1,
        [&]() {
          if (_actions.size() >= 2U) {
            if (cider::recorder::semanticallyEqual(
                    _actions[_actions.size() - 1],
                    _actions[_actions.size() - 2])) {
              if (_actions.size() >= 3U) {
                if (cider::recorder::semanticallyEqual(
                        _actions[_actions.size() - 2],
                        _actions[_actions.size() - 3])) {
                  _rwCounter.threeSameAct++;
                  return -0.2;
                }
              }
              _rwCounter.twoSameAct++;
              return -0.15;
            }
          }

          _rwCounter.sameCov++;
          return -0.1;
        },
        0.1);
  }

  if (result.has_value()) {
    _lastObjVal = objValue;

    const double lenghtMultiplier = calculateContinuousMultiplier(
        _rwCounter, _initialSize, _actions.size());

    std::cout << "Multi: " << lenghtMultiplier << ", old: " << _initialSize
              << ", new: " << _actions.size() << std::endl;

    result = normalize_reward(result.value() * lenghtMultiplier);
  }

  return result;
}

}  // namespace qleaning
}  // namespace cider
