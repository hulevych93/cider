// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"
#include "greedy-r/greedy-r.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

class GreedyRStage final : public Pipe {
 public:
  GreedyRStage(const greedy_r::GreedyRSettings& settings, int numberOfRuns = 1);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "GREEDY_R"; }

 private:
  std::string getConfigName() const;

  greedy_r::GreedyRSettings m_settings;
  const int _numberOfRuns;
};

}  // namespace pipelines
}  // namespace cider
