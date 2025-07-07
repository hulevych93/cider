// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "agent.h"
#include "logger.h"
#include "reward_counter.h"

namespace cider {
namespace qleaning {

using ObjectiveFunction =
    std::function<ObjectiveValue(const std::vector<recorder::Action>&)>;

struct LearningSettings final {
  double learningRate = 0.1;
  double discountFactor = 0.9;
  size_t episodes = 50U;
  size_t maxRollback = 10U;
  size_t maxStateDepth = 10U;
  ObjectiveFunction objFunc;
};

std::ostream& operator<<(std::ostream& os, const LearningSettings& settings);

void prelearningSession(const LearningSettings& settings,
                        const recorder::Actions& list,
                        QAgent& agent);

void learningSession(RewardCounter& counter,
                     const LearningSettings& settings,
                     const recorder::Actions& list,
                     QAgent& agent,
                     IResultsLogger& logger,
                     const std::function<void()>& dump);

}  // namespace qleaning
}  // namespace cider
