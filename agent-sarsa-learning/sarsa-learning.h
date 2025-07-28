// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "agent-model/logger.h"
#include "agent-model/reward_counter.h"
#include "agent-model/settings.h"
#include "sarsa-learning-agent.h"

namespace cider {
namespace agent_model {
namespace sarsa {

using ObjectiveFunction =
    std::function<ObjectiveValue(const std::vector<recorder::Action>&)>;

void prelearningSession(const SarsaLearningSettings& settings,
                        const recorder::Actions& list,
                        ObjectiveFunction objFunc,
                        IResultsLogger& logger);

void learningSession(const SarsaLearningSettings& settings,
                     const recorder::Actions& list,
                     ObjectiveFunction objFunc,
                     RewardCounter& counter,
                     IResultsLogger& logger,
                     ICovLogger& covLogger,
                     const std::function<void(const IAgent&)>& dump);

SarsaLearningAgent& getAgent(const SarsaLearningSettings& settings);

}  // namespace sarsa
}  // namespace agent_model
}  // namespace cider
