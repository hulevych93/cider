// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "agent-synthesis.h"

#include "agent-model/scenario.h"

#include "agent-q-learning/q-learning-agent.h"
#include "agent-sarsa-learning/sarsa-learning-agent.h"

#include <tlog.h>

#include <assert.h>

namespace cider {
namespace synthesis {

bool synthesize(std::mt19937& gen,
                const QSynthesisSettings& settings,
                ObjectiveFunction objFunc,
                FineObjectiveFunction fineObjFunc,
                const recorder::Actions& initial,
                recorder::Actions& out) {
  const auto& agent = agent_model::qlearning::QLearningAgent::get();
  return synthesize(gen, settings, agent, objFunc, fineObjFunc, initial, out);
}

bool synthesize(std::mt19937& gen,
                const SarsaSynthesisSettings& settings,
                ObjectiveFunction objFunc,
                FineObjectiveFunction fineObjFunc,
                const recorder::Actions& initial,
                recorder::Actions& out) {
  const auto& agent = agent_model::sarsa::SarsaLearningAgent::get();
  return synthesize(gen, settings, agent, objFunc, fineObjFunc, initial, out);
}

bool synthesize(std::mt19937& gen,
                const AgentSynthesisSettings& settings,
                const agent_model::IAgent& agent,
                ObjectiveFunction objFunc,
                FineObjectiveFunction fineObjFunc,
                const recorder::Actions& initial,
                recorder::Actions& out) {
  out.clear();

  const auto maxStateDepth = agent.getMaxStateDepth();
  assert(maxStateDepth > 0);

  agent_model::Scenario scenario(gen, maxStateDepth, initial, objFunc,
                                 fineObjFunc);

  const auto actionChoosing = [&](const synthesis::TestScenario& testCase) {
    const auto& scenario = dynamic_cast<const agent_model::Scenario&>(testCase);

    std::optional<recorder::Action> selectedOpt;

    switch (settings.strategy) {
      case GenerationStrategyType::Greedy:
        selectedOpt = agent.chooseGreedyAction(scenario);
        break;
      case GenerationStrategyType::EGreedy:
        selectedOpt = agent.chooseEGreedyAction(scenario, settings.epsilon);
        break;
      case GenerationStrategyType::Boltzmann:
        selectedOpt = agent.chooseBolzmanAction(scenario, settings.temperature);
        break;
    }

    return selectedOpt;
  };

  details::synthesize(settings, actionChoosing, scenario);

  out = scenario.getResult();

  return true;
}

}  // namespace synthesis
}  // namespace cider
