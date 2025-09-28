// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipes/simulation/simulation-pipe.h"

#include "coverage/coverage.h"

#include "synthesis/synthesis.h"

namespace cider {
namespace pipelines {

class SynthesisStage final : public SimulationPipe {
 public:
  SynthesisStage(const synthesis::SynthesisSettings& settings,
                 int numberOfRuns = 1);

  std::string getLetter() const override { return "G"; }

  bool needTS() const override { return true; }

 private:
  bool simulate(const std::string& outPath,
                const double baseline,
                const recorder::Actions& input,
                recorder::Actions& output,
                const ObjectiveFunction& objFunc,
                const ObjectiveFunction& objFuncСfg,
                const FineObjectiveFunction& fineObjFunc) override;

  std::string getConfigName() const override;
  std::string getPrefix() const override;

  synthesis::SynthesisSettings m_settings;
};

}  // namespace pipelines
}  // namespace cider
