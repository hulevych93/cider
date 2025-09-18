// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "greedy-r-pipe.h"

namespace cider {
namespace pipelines {

GreedyRStage::GreedyRStage(const greedy_r::GreedyRSettings& settings,
                           int numberOfRuns)
    : SimulationPipe(numberOfRuns), m_settings(settings) {}

bool GreedyRStage::simulate(const std::string& /*outPath*/,
                            const double /*baseline*/,
                            const recorder::Actions& input,
                            recorder::Actions& output,
                            const ObjectiveFunction& objFunc,
                            const FineObjectiveFunction& /*fineObjFunc*/) {
  m_settings.objFunc = objFunc;
  output = greedy_r::run_greedy_r(Seed::instance().get(), m_settings, input);
  return true;
}

std::string GreedyRStage::getPrefix() const {
  std::stringstream os;
  os << m_settings;
  return os.str();
}

std::string GreedyRStage::getConfigName() const {
  return m_settings.configName;
}

}  // namespace pipelines
}  // namespace cider
