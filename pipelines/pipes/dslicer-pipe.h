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

class DQLPostProcessSlicerStage final : public Pipe {
 public:
  explicit DQLPostProcessSlicerStage(const ReportConfiguration& config);

 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "QLEG_QLB_DSL"; }

  bool needTS() const override { return true; }

 private:
  ReportConfiguration _config;
};

}  // namespace pipelines
}  // namespace cider
