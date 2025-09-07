// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "cov-box-plot-report-pipe.h"

#include "mathplot-log/output/comp-coverage-box-plot.h"

namespace cider {
namespace pipelines {

BoxPlotReportStage::BoxPlotReportStage(const ReportConfiguration& config)
    : GraphReportStage("CBOX", config) {}

std::shared_ptr<cider::mathplot::IBasicPlot> BoxPlotReportStage::makePlot(
    const std::string& libName,
    const std::string& logDir,
    const std::string& logFileName) {
  return std::make_shared<cider::mathplot::CoverageBoxPlot>(
      libName, logDir, logFileName,
      cider::mathplot::CoverageBoxPlot::PlotType::BrCov);
}

std::function<void(cider::mathplot::IBasicPlot* plot,
                   const std::string& methodName,
                   const std::string& libName,
                   const cider::Cmd& cmd,
                   const Result& result)>
BoxPlotReportStage::createProcessor() {
  return
      [](cider::mathplot::IBasicPlot* plot, const std::string& methodName,
         const std::string& libName, const cider::Cmd&, const Result& result) {
        auto* logger = dynamic_cast<cider::mathplot::CoverageBoxPlot*>(plot);

        logger->log(methodName, result.newReport);
        logger->setOriginalCov(getOldCov(libName, result.oldReport));
      };
}

}  // namespace pipelines
}  // namespace cider
