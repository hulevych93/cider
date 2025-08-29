// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"
#include "monte-carlo-tree-search/monte-carlo.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

class MctsSearchStage final : public Pipe {
 public:
  MctsSearchStage(const mcts::MonteCarloSettings& settings,
                  int numberOfRuns = 1);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "MCTS"; }

  bool needTS() const override { return false; }

 private:
  std::string getConfigName() const;

  mcts::MonteCarloSettings m_settings;
  const int _numberOfRuns;
};

}  // namespace pipelines
}  // namespace cider
