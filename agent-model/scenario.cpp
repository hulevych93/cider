#include "scenario.h"

#include <tlog.h>

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

}  // namespace

Scenario::Scenario(std::mt19937& gen,
                   int maxStateDepth,
                   const recorder::Actions& initial,
                   const synthesis::ObjectiveFunction& objFunc)
    : synthesis::TestScenario(gen, initial, objFunc),
      _maxStateDepth(maxStateDepth) {}

recorder::Actions Scenario::getCurrentState() const {
  auto count = _maxStateDepth;
  if (count > _actions.size()) {
    count = _actions.size();
  }

  return recorder::Actions{_actions.end() - count, _actions.end()};
}

LearningScenario::LearningScenario(RewardCounter& counter,
                                   std::mt19937& gen,
                                   int maxStateDepth,
                                   const recorder::Actions& initial,
                                   const synthesis::ObjectiveFunction& objFunc)
    : Scenario(gen, maxStateDepth, initial, objFunc), _rwCounter(counter) {}

std::optional<double> LearningScenario::getReward() const {
  const auto objValue = _objFunc(_actions);

  std::optional<double> result;

  if (synthesis::isOverFunc(objValue, _initialObjVal,
                            _availableActions.empty())) {
    result = rewardFunction(
        _rwCounter, objValue, _initialObjVal, finalReward.coverageIncreased,
        finalReward.newTracksFound,
        [&]() {
          if (_initialSize > _actions.size()) {
            return finalReward.sameButShorter;
          } else {
            return finalReward.sameCoverage;
          }
        },
        finalReward.lowerCoverage);
  } else {
    result = rewardFunction(
        _rwCounter, objValue, _lastObjVal, stepReward.coverageIncreased,
        stepReward.newTracksFound,
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
                  return stepReward.threeSemanticallyEqualAction;
                }
              }
              _rwCounter.twoSameAct++;
              return stepReward.twoSemanticallyEqualAction;
            }
          }

          _rwCounter.sameCov++;
          return stepReward.sameCoverage;
        },
        stepReward.lowerCoverage);
  }

  if (result.has_value()) {
    _lastObjVal = objValue;

    const double lenghtMultiplier = calculateContinuousMultiplier(
        _rwCounter, _initialSize, _actions.size());

    tlog_info << "Multi: " << lenghtMultiplier << ", old: " << _initialSize
              << ", new: " << _actions.size() << std::endl;

    result = normalize_reward(result.value() * lenghtMultiplier);
  }

  return result;
}

double LearningScenario::getCoverage(bool retry) const {
  auto cov = _objFunc(_actions).coverage;
  if (cov < 0.0000001 && retry) {
    return _objFunc(_actions).coverage;
  }
  return cov;
}

std::string actionToGenericRepro(const recorder::Action& action) {
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

std::string actionsToGenericRepro(
    const std::vector<recorder::Action>& actions) {
  if (actions.empty()) {
    return "Empty";
  }
  std::string result;
  for (const auto& action : actions) {
    result += actionToGenericRepro(action) + ';';
  }
  result.pop_back();
  return result;
}

}  // namespace agent_model
}  // namespace cider
