// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-synthesis.h"

#include "q-learning/qtable-agent.h"
#include "q-learning/scenario.h"

#include <iostream>

#include <assert.h>

namespace cider {
namespace synthesis {

std::ostream& operator<<(std::ostream& os, const QSynthesisSettings& settings) {
  os << "QL_GEN_";
  std::string strategyName;
  std::string stopType;

  switch (settings.strategy) {
    case GenerationStrategyType::Greedy:
      strategyName = "Greedy";
      break;
    case GenerationStrategyType::EGreedy:
      strategyName = "E-Greedy";
      break;
    case GenerationStrategyType::Boltzmann:
      strategyName = "Boltzmann";
      break;
    default:
      strategyName = "Unknown";
  }

  switch (settings.stopType) {
    case synthesis::StopCondition::LimitActions:
      stopType = "LimitActions";
      break;
    case synthesis::StopCondition::GreaterCoverage:
      stopType = "GreaterCoverage";
      break;
    default:
      strategyName = "Unknown";
  }

  os << "st[" << strategyName;
  os << "]_eps[" << settings.epsilon;
  os << "]_temp[" << settings.temperature;
  os << "]_maxSt[" << settings.maxRollback;
  os << "]_stType[" << stopType;
  os << "]_lim[" << settings.limitActions;
  os << "]";
  return os;
}

bool synthesize(std::mt19937& gen,
                const QSynthesisSettings& settings,
                ObjectiveFunction objFunc,
                const recorder::Actions& initial,
                recorder::Actions& out) {
  out.clear();

  auto& agent = qleaning::getAgent();

  qleaning::Scenario scenario(gen, settings.maxStateDepth, initial, objFunc);

  const auto actionChoosing = [&](const synthesis::TestScenario& testCase) {
    const auto& scenario = dynamic_cast<const qleaning::Scenario&>(testCase);

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
