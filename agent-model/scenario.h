// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "agent-model/reward_counter.h"
#include "synthesis/test-case.h"

#include "reward_func.h"

namespace cider {
namespace agent_model {

class Scenario : public synthesis::TestScenario {
 public:
  Scenario(std::mt19937& gen,
           int maxStateDepth,
           const recorder::Actions& initial,
           const synthesis::ObjectiveFunction& objFunc);

  recorder::Actions getCurrentState() const;

 private:
  int _maxStateDepth = 0;
};

class LearningScenario : public Scenario {
 public:
  LearningScenario(const RewardShappingParams& params,
                   RewardCounter& counter,
                   std::mt19937& gen,
                   int maxStateDepth,
                   const recorder::Actions& initial,
                   const synthesis::ObjectiveFunction& objFunc);

  std::optional<double> getReward() const;

  size_t calculateRedundancy(bool local) const;

  double getCoverage(bool retry) const;

 protected:
  RewardCounter& _rwCounter;
  const RewardShappingParams _params;
};

std::string actionToGenericRepro(const recorder::Action& action);
std::string actionsToGenericRepro(const std::vector<recorder::Action>& actions);

}  // namespace agent_model
}  // namespace cider
