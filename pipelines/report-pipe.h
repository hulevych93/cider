// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "coverage/coverage.h"
#include "recorder/recorder.h"

#include "q-learning/q-learning.h"

namespace cider {
namespace pipelines {

class StepperReportStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "RP"; }
};

class BoxPlotReportStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "BOX_PLOT"; }
};

class CovBarPlotReportStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "COV_BAR_PL"; }
};

class LinesBarPlotReportStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "LN_BAR_PL"; }
};

}  // namespace pipelines
}  // namespace cider
