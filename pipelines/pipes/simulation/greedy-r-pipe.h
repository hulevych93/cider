// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipes/simulation/simulation-pipe.h"

#include "coverage/coverage.h"
#include "greedy-r/greedy-r.h"

namespace cider {
namespace pipelines {

class GreedyRStage final : public SimulationPipe {
 public:
  GreedyRStage(const greedy_r::GreedyRSettings& settings, int numberOfRuns);

  std::string getLetter() const override { return "GREEDY_R"; }

 private:
  bool simulate(const std::string& outPath,
                const double baseline,
                const recorder::Actions& input,
                recorder::Actions& output,
                const ObjectiveFunction& objFunc,
                const FineObjectiveFunction& fineObjFunc) override;

  std::string getConfigName() const override;
  std::string getPrefix() const override;

  greedy_r::GreedyRSettings m_settings;
};

}  // namespace pipelines
}  // namespace cider
