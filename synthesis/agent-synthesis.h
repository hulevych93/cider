// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "agent-model/agent.h"
#include "synthesis/synthesis.h"

namespace cider {
namespace synthesis {

bool synthesize(std::mt19937& gen,
                const QSynthesisSettings& settings,
                ObjectiveFunction objFunc,
                FineObjectiveFunction fineObjFunc,
                const recorder::Actions& initial,
                recorder::Actions& out);

bool synthesize(std::mt19937& gen,
                const SarsaSynthesisSettings& settings,
                ObjectiveFunction objFunc,
                FineObjectiveFunction fineObjFunc,
                const recorder::Actions& initial,
                recorder::Actions& out);

bool synthesize(std::mt19937& gen,
                const AgentSynthesisSettings& settings,
                const agent_model::IAgent& agent,
                ObjectiveFunction objFunc,
                FineObjectiveFunction fineObjFunc,
                const recorder::Actions& initial,
                recorder::Actions& out);

}  // namespace synthesis
}  // namespace cider
