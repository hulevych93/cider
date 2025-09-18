// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipes/simulation/simulation-pipe.h"

#include "coverage/coverage.h"
#include "metaheuristics/metasearch.h"

namespace cider {
namespace pipelines {

class MetaSearchStage final : public SimulationPipe {
 public:
  MetaSearchStage(const metasearch::MetaSettings& settings,
                  int numberOfRuns = 1);

  std::string getLetter() const override { return "HS"; }

 private:
  bool simulate(const std::string& outPath,
                const double baseline,
                const recorder::Actions& input,
                recorder::Actions& output,
                const ObjectiveFunction& objFunc,
                const FineObjectiveFunction& fineObjFunc) override;

  std::string getConfigName() const override;
  std::string getPrefix() const override;

  metasearch::MetaSettings m_settings;
};

}  // namespace pipelines
}  // namespace cider
