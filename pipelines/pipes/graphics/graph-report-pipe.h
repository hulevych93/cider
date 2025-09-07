// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"

#include "mathplot-log/output/basic-plot.h"

namespace cider {
namespace pipelines {

class QLearningReportStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }
};

class GraphReportStage : public Pipe {
 public:
  GraphReportStage(const std::string& title, const ReportConfiguration& config);

 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }

  virtual std::shared_ptr<cider::mathplot::IBasicPlot> makePlot(
      const std::string& libName,
      const std::string& logDir,
      const std::string& logFileName) = 0;

  virtual std::function<void(cider::mathplot::IBasicPlot* plot,
                             const std::string& methodName,
                             const std::string& libName,
                             const cider::Cmd& cmd,
                             const Result& result)>
  createProcessor() = 0;

 private:
  std::string _title;
  ReportConfiguration _config;
};

}  // namespace pipelines
}  // namespace cider
