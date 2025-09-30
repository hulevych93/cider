// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "agent-model/reward_counter.h"
#include "coverage/coverage.h"

namespace cider {
namespace agent_model {

static struct RewardShappingParams final {
  double alpha = 1.5;
  double beta = 0.3;
  double gamma = 0.6;
  const double finalFactor = 10.0;
  const double rewardBaseValue = 0.5;
} rewardShaping;

std::optional<double> getShapedReward(const RewardShappingParams& params,
                                      RewardCounter& rw,
                                      const ObjectiveValue& objValue,
                                      ObjectiveValue& prevObjValue,
                                      const ObjectiveValue& initialObjValue,
                                      size_t initLen,
                                      size_t currentLen,
                                      size_t redundancy,
                                      const bool hasActions);

}  // namespace agent_model
}  // namespace cider
