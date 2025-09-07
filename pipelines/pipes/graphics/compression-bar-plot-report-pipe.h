// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"

#include "pipelines/pipes/graphics/graph-report-pipe.h"

namespace cider {
namespace pipelines {

class CompressionBarPlotReportStage final : public GraphReportStage {
 public:
  explicit CompressionBarPlotReportStage(const ReportConfiguration& config);

 protected:
  std::shared_ptr<cider::mathplot::IBasicPlot> makePlot(
      const std::string& libName,
      const std::string& logDir,
      const std::string& logFileName) override;

  std::function<void(cider::mathplot::IBasicPlot* plot,
                     const std::string& methodName,
                     const std::string& libName,
                     const cider::Cmd& cmd,
                     const Result& result)>
  createProcessor() override;
};

}  // namespace pipelines
}  // namespace cider
