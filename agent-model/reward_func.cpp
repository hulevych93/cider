#include "reward_func.h"

#include <tlog.h>

#include "synthesis/test-case.h"

namespace cider {
namespace agent_model {

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

    tlog_info << objValue.coverage << std::endl;

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
      return penalty;
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

struct RewardValue final {
  double coverageIncreased =
      rewardShaping.rewardBaseValue * rewardShaping.alpha;
  double newTracksFound = rewardShaping.rewardBaseValue * rewardShaping.alpha;
  double twoSemanticallyEqualAction =
      (-1) * rewardShaping.rewardBaseValue * rewardShaping.beta;
  double threeSemanticallyEqualAction =
      (-1) * rewardShaping.rewardBaseValue * rewardShaping.beta * 2;
  double sameButShorter = rewardShaping.rewardBaseValue * rewardShaping.gamma;
  double sameCoverage =
      (rewardShaping.rewardBaseValue * rewardShaping.gamma) / 2;
  double lowerCoverage =
      (-1) * rewardShaping.rewardBaseValue * rewardShaping.alpha;
};

auto getRewardValue(const RewardShappingParams& shaping) {
  RewardValue rewardValue;
  rewardValue.coverageIncreased = shaping.rewardBaseValue * shaping.alpha;
  rewardValue.newTracksFound = shaping.rewardBaseValue * shaping.alpha;
  rewardValue.twoSemanticallyEqualAction =
      (-1) * shaping.rewardBaseValue * shaping.beta;
  rewardValue.threeSemanticallyEqualAction =
      (-1) * shaping.rewardBaseValue * shaping.beta * 2;
  rewardValue.sameButShorter = shaping.rewardBaseValue * shaping.gamma;
  rewardValue.sameCoverage = (shaping.rewardBaseValue * shaping.gamma) / 2;
  rewardValue.lowerCoverage = (-1) * shaping.rewardBaseValue * shaping.alpha;
  return rewardValue;
}

}  // namespace

std::optional<double> getShapedReward(const RewardShappingParams& rewardShaping,
                                      RewardCounter& rw,
                                      const ObjectiveValue& objValue,
                                      ObjectiveValue& prevObjValue,
                                      const ObjectiveValue& initialObjValue,
                                      size_t initLen,
                                      size_t currentLen,
                                      size_t redundancy,
                                      const bool hasActions) {
  std::optional<double> result;

  const auto rewardValue = getRewardValue(rewardShaping);

  if (synthesis::isOverFunc(objValue, initialObjValue, !hasActions)) {
    result = rewardFunction(
        rw, objValue, initialObjValue,
        rewardValue.coverageIncreased * rewardShaping.finalFactor,
        rewardValue.newTracksFound * rewardShaping.finalFactor,
        [&]() {
          if (initLen > currentLen) {
            return rewardValue.sameButShorter * rewardShaping.finalFactor;
          } else {
            return rewardValue.sameCoverage * rewardShaping.finalFactor;
          }
        },
        rewardValue.lowerCoverage * rewardShaping.finalFactor);
  } else {
    result = rewardFunction(
        rw, objValue, prevObjValue, rewardValue.coverageIncreased,
        rewardValue.newTracksFound,
        [&]() {
          if (redundancy == 2) {
            rw.threeSameAct++;
            return rewardValue.threeSemanticallyEqualAction;
          } else if (redundancy == 1) {
            rw.twoSameAct++;
            return rewardValue.twoSemanticallyEqualAction;
          }

          if (initLen > currentLen) {
            return rewardValue.sameButShorter;
          } else {
            return rewardValue.sameCoverage;
          }
        },
        rewardValue.lowerCoverage);
  }

  if (result.has_value()) {
    prevObjValue = objValue;

    // const double lenghtMultiplier =
    //   calculateContinuousMultiplier(rw, initLen, currentLen);

    // tlog_info << "lenghtMultiplier: " << lenghtMultiplier << std::endl;

    result = normalize_reward(result.value());
  }

  return result;
}

}  // namespace agent_model
}  // namespace cider
