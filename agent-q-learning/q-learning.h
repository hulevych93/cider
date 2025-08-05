// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "agent-model/agent.h"
#include "agent-model/logger.h"
#include "agent-model/reward_counter.h"
#include "agent-model/settings.h"

namespace cider {
namespace agent_model {
namespace qlearning {

using ObjectiveFunction =
    std::function<ObjectiveValue(const std::vector<recorder::Action>&)>;

void prelearningSession(const QLearningSettings& settings,
                        const recorder::Actions& list,
                        ObjectiveFunction objFunc,
                        IRewardLogger& rwLogger,
                        ILossLogger& lossLogger);

void learningSession(const QLearningSettings& settings,
                     const recorder::Actions& list,
                     ObjectiveFunction objFunc,
                     RewardCounter& counter,
                     IRewardLogger& rwLogger,
                     ILossLogger& lossLogger,
                     ICoverageLogger& covLogger,
                     const std::function<void(const IAgent&)>& dump);

}  // namespace qlearning
}  // namespace agent_model
}  // namespace cider
