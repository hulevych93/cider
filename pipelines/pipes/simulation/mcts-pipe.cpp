// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "mcts-pipe.h"

namespace cider {
namespace pipelines {

MctsSearchStage::MctsSearchStage(const mcts::MonteCarloSettings& settings,
                                 int numberOfRuns)
    : SimulationPipe(numberOfRuns), m_settings(settings) {}

bool MctsSearchStage::simulate(const std::string& /*outPath*/,
                               const double /*baseline*/,
                               const recorder::Actions& input,
                               recorder::Actions& output,
                               const ObjectiveFunction& objFunc,
                               const ObjectiveFunction& /*objFuncСfg*/,
                               const FineObjectiveFunction& /*fineObjFunc*/) {
  m_settings.objFunc = objFunc;
  output = mcts::run_mcts(Seed::instance().get(), m_settings, input);
  return true;
}

std::string MctsSearchStage::getPrefix() const {
  std::stringstream os;
  os << m_settings;
  return os.str();
}

std::string MctsSearchStage::getConfigName() const {
  return m_settings.configName;
}

}  // namespace pipelines
}  // namespace cider
