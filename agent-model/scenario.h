// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "agent-model/reward_counter.h"
#include "synthesis/test-case.h"

namespace cider {
namespace agent_model {

static struct StepRewardSettings final {
  double coverageIncreased = 0.5;
  double newTracksFound = 0.5;
  double twoSemanticallyEqualAction = -0.15;
  double threeSemanticallyEqualAction = -0.3;
  double sameCoverage = 0.05;
  double lowerCoverage = -0.5;
} stepReward;

static struct FinalRewardSettings final {
  double coverageIncreased = 5.0;
  double newTracksFound = 5.0;
  double sameButShorter = 3.0;
  double sameCoverage = 0.5;
  double lowerCoverage = -5.0;
} finalReward;

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

class LearningScenario final : public Scenario {
 public:
  LearningScenario(RewardCounter& counter,
                   std::mt19937& gen,
                   int maxStateDepth,
                   const recorder::Actions& initial,
                   const synthesis::ObjectiveFunction& objFunc);

  std::optional<double> getReward() const;

  double getCoverage(bool retry) const;

 private:
  RewardCounter& _rwCounter;
};

std::string actionToGenericRepro(const recorder::Action& action);
std::string actionsToGenericRepro(const std::vector<recorder::Action>& actions);

}  // namespace agent_model
}  // namespace cider
