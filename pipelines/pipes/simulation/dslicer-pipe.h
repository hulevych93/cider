// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipes/simulation/simulation-pipe.h"

#include "coverage/coverage.h"
#include "dynamic-slicer/dynamic-slicer.h"

namespace cider {
namespace pipelines {

class DSlicerStage final : public SimulationPipe {
 public:
  explicit DSlicerStage(const dslicer::DSLSettings& settings, int numberOfRuns);

  std::string getLetter() const override { return "DSLICER"; }

 private:
  bool simulate(const std::string& outPath,
                const double baseline,
                const recorder::Actions& input,
                recorder::Actions& output,
                const ObjectiveFunction& objFunc,
                const ObjectiveFunction& objFuncСfg,
                const FineObjectiveFunction& fineObjFunc) override;

  std::string getConfigName() const override;
  std::string getPrefix() const override;

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

}  // namespace pipelines
}  // namespace cider
