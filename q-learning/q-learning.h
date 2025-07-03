// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "agent.h"
#include "logger.h"
#include "reward_counter.h"
#include "scenario.h"
#include "settings.h"

namespace cider {
namespace qleaning {

void prelearningSession(const LearningSettings& settings,
                        const QActionList& list,
                        QAgent& agent);

void learningSession(RewardCounter& counter,
                     const LearningSettings& settings,
                     const QActionList& list,
                     QAgent& agent,
                     IResultsLogger& logger,
                     const std::function<void()>& dump);

bool gererationSession(const GenerationSettings& settings,
                       const QAgent& agent,
                       const QActionList& initial,
                       QActionList& out);

QAgent& getAgent(const std::string& path = "");

}  // namespace qleaning
}  // namespace cider
