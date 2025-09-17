// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"
#include "dynamic-slicer/dynamic-slicer.h"

namespace cider {
namespace pipelines {

class DSlicerStage final : public Pipe {
 public:
  explicit DSlicerStage(const dslicer::DSLSettings& settings);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "DSLICER"; }

 private:
  dslicer::DSLSettings _settings;
};

class DQLPostProcessSlicerStage final : public Pipe {
 public:
  DQLPostProcessSlicerStage(const dslicer::DSLSettings& settings,
                            const ReportConfiguration& config);

 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "QLEG_QLB_DSL"; }

  bool needTS() const override { return true; }

 private:
  ReportConfiguration _config;
  dslicer::DSLSettings _settings;
};

class FastDQLPostProcessSlicerStage final : public Pipe {
 public:
  explicit FastDQLPostProcessSlicerStage(const ReportConfiguration& config);

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
