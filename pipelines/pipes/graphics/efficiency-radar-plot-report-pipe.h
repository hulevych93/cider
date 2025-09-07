// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "pipelines/pipeline.h"

#include "pipelines/pipes/graphics/graph-report-pipe.h"

#include "math/stat-utils.h"

namespace cider {
namespace pipelines {

class EfficiencyRadarPlotReportStage final : public GraphReportStage {
 public:
  explicit EfficiencyRadarPlotReportStage(const ReportConfiguration& config);

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

 private:
  math_stat::FilterManager _filter;
  math_stat::FilterManager _filter1;
  math_stat::FilterManager _filter2;
};

}  // namespace pipelines
}  // namespace cider
