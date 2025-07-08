// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "coverage/coverage.h"

#include <variant>

namespace cider {
namespace agent_model {

struct LearningSettingsBase {
  const char* configName = "L_NAN";
  double learningRate = 0.1;
  double discountFactor = 0.9;
  size_t episodes = 50U;
  size_t maxRollback = 10U;
  size_t maxStateDepth = 10U;
};

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
