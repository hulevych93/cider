// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "coverage/coverage.h"

#include "reward_func.h"

#include <variant>

namespace cider {
namespace agent_model {

struct LearningSettingsBase {
  const char* configName = "L_NAN";
  double initialLearningRate = 0.3;
  double finalLearningRate = 0.1;
  double discountFactor = 0.85;
  size_t prelearningEpisodes = 0U;
  size_t episodes = 50U;
  size_t maxRollback = 20U;
  size_t maxStateDepth = 5U;
  size_t coverageConvergenceCounter = 200U;

  RewardShappingParams rewardShaping;
};

double linearDecay(const double initial,
                   const double final,
                   const int eps,
                   const int ep);

namespace qlearning {
struct QLearningSettings final : LearningSettingsBase {};
}  // namespace qlearning

namespace sarsa {
struct SarsaLearningSettings final : LearningSettingsBase {};
}  // namespace sarsa

using LearningSettings =
    std::variant<qlearning::QLearningSettings, sarsa::SarsaLearningSettings>;

std::ostream& operator<<(std::ostream& os,
                         const LearningSettingsBase& settings);

}  // namespace agent_model
}  // namespace cider
