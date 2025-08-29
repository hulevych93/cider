// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"

namespace cider {
namespace pipelines {

class ResultsAgregationStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "AGGR"; }

  bool needTS() const override { return true; }
};

}  // namespace pipelines
}  // namespace cider
