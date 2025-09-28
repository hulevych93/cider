// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "synthesis-pipe.h"

#include "synthesis/agent-synthesis.h"
#include "synthesis/rand-synthesis.h"

#include <assert.h>
#include <tlog.h>

namespace cider {
namespace pipelines {

SynthesisStage::SynthesisStage(const synthesis::SynthesisSettings& settings,
                               int numberOfRuns)
    : SimulationPipe(numberOfRuns), m_settings(settings) {}

bool SynthesisStage::simulate(const std::string& /*outPath*/,
                              const double /*baseline*/,
                              const recorder::Actions& input,
                              recorder::Actions& output,
                              const ObjectiveFunction& objFunc,
                              const ObjectiveFunction& /*objFuncСfg*/,
                              const FineObjectiveFunction& /*fineObjFunc*/) {
  return std::visit(
      [&](const auto& s) {
        return synthesis::synthesize(Seed::instance().get(), s, objFunc, input,
                                     output);
      },
      m_settings);
}

std::string SynthesisStage::getPrefix() const {
  std::stringstream os;
  std::visit([&os](const auto& s) { os << "_" << s; }, m_settings);
  return os.str();
}

std::string SynthesisStage::getConfigName() const {
  return std::visit([&](const auto& settings) { return settings.configName; },
                    m_settings);
}

}  // namespace pipelines
}  // namespace cider
