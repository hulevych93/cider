// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipes/simulation/simulation-pipe.h"

#include "coverage/coverage.h"
#include "monte-carlo-tree-search/monte-carlo.h"

namespace cider {
namespace pipelines {

class MctsSearchStage final : public SimulationPipe {
 public:
  MctsSearchStage(const mcts::MonteCarloSettings& settings,
                  int numberOfRuns = 1);

  std::string getLetter() const override { return "MCTS"; }

  bool needTS() const override { return true; }

 private:
  bool simulate(const std::string& outPath,
                const double baseline,
                const recorder::Actions& input,
                recorder::Actions& output,
                const ObjectiveFunction& objFunc,
                const FineObjectiveFunction& fineObjFunc) override;

  std::string getConfigName() const override;
  std::string getPrefix() const override;

  mcts::MonteCarloSettings m_settings;
};

}  // namespace pipelines
}  // namespace cider
