// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipes/simulation/simulation-pipe.h"

#include "coverage/coverage.h"

#include "synthesis/synthesis.h"

namespace cider {

namespace mathplot {
class SuffixFreqPlot;
}

namespace pipelines {

class SynthesisStage final : public SimulationPipe {
 public:
  SynthesisStage(const synthesis::SynthesisSettings& settings,
                 int numberOfRuns = 1);
  ~SynthesisStage();

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

  void onCleanup() override;

  std::string getConfigName() const override;
  std::string getPrefix() const override;

  synthesis::SynthesisSettings m_settings;

  std::unique_ptr<mathplot::SuffixFreqPlot> _freqPlot;
};

}  // namespace pipelines
}  // namespace cider
