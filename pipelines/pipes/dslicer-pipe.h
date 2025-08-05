// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"
#include "dynamic-slicer/dynamic-slicer.h"

namespace cider {
namespace pipelines {

class DSlicerStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "DSLICER"; }
};

}  // namespace pipelines
}  // namespace cider
