// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "pipelines/pipeline.h"

#include "coverage/cfg_measurer.h"
#include "coverage/coverage.h"
#include "coverage/gcov_measurer.h"

namespace cider {
namespace pipelines {

using ObjectiveFunction =
    std::function<ObjectiveValue(const recorder::Actions&)>;

using FineObjectiveFunction =
    std::function<FineObjectiveValue(const recorder::Actions&)>;

class SimulationPipe : public Pipe {
 public:
  explicit SimulationPipe(int numberOfRuns = 1);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override final;

  bool needTS() const override { return false; }

 private:
  virtual bool simulate(const std::string& outPath,
                        const double baseline,
                        const recorder::Actions& input,
                        recorder::Actions& output,
                        const ObjectiveFunction& objFunc,
                        const ObjectiveFunction& objFuncСfg,
                        const FineObjectiveFunction& fineObjFunc) = 0;

  virtual std::string getConfigName() const = 0;
  virtual std::string getPrefix() const = 0;

  const int _numberOfRuns;
};

}  // namespace pipelines
}  // namespace cider
