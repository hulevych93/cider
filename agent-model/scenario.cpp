#include "scenario.h"

#include "reward_func.h"

#include <tlog.h>

namespace cider {
namespace agent_model {

recorder::Actions takeSuffix(const recorder::Actions& actions, size_t k) {
  if (k == 0 || actions.empty())
    return {};
  if (k >= actions.size())
    return actions;
  return recorder::Actions(actions.end() - static_cast<long>(k), actions.end());
}

Scenario::Scenario(std::mt19937& gen,
                   int maxStateDepth,
                   const recorder::Actions& initial,
                   const synthesis::ObjectiveFunction& objFunc,
                   const synthesis::FineObjectiveFunction& fineObjFunc)
    : synthesis::TestScenario(gen, initial, objFunc, fineObjFunc),
      _maxStateDepth(maxStateDepth) {}

recorder::Actions Scenario::getCurrentState() const {
  return takeSuffix(_actions, _maxStateDepth);
}

LearningScenario::LearningScenario(
    const RewardShappingParams& params,
    RewardCounter& counter,
    std::mt19937& gen,
    int maxStateDepth,
    const recorder::Actions& initial,
    const synthesis::ObjectiveFunction& objFunc,
    const synthesis::FineObjectiveFunction& fineObjFunc)
    : Scenario(gen, maxStateDepth, initial, objFunc, fineObjFunc),
      _rwCounter(counter),
      _params(params) {}

std::optional<double> LearningScenario::getReward() const {
  const auto objValue = _objFunc(_actions);

  return getShapedReward(_params, _rwCounter, objValue, _lastObjVal,
                         _initialObjVal, _initialSize, _actions.size(),
                         calculateRedundancy(true), !_availableActions.empty());
}

size_t LearningScenario::calculateRedundancy(bool local) const {
  if (_actions.size() < 2)
    return 0;

  const auto getRedundancy = [](const recorder::Actions& actions) {
    int redundancy = 0;

    for (size_t i = 1; i < actions.size(); ++i) {
      if (cider::recorder::semanticallyEqual(actions[i], actions[i - 1])) {
        redundancy++;
      }
    }
    return redundancy;
  };

  if (local) {
    return getRedundancy(getCurrentState());
  } else {
    return getRedundancy(_actions);
  }
}

double LearningScenario::getCoverage(bool retry) const {
  auto cov = _objFunc(_actions).coverage;
  if (cov < 0.0000001 && retry) {
    return _objFunc(_actions).coverage;
  }
  return cov;
}

}  // namespace agent_model
}  // namespace cider
