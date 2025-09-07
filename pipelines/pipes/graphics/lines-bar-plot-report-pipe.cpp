// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "lines-bar-plot-report-pipe.h"

#include "mathplot-log/output/comp-lines-barplot.h"

namespace cider {
namespace pipelines {

LinesBarPlotReportStage::LinesBarPlotReportStage(
    const ReportConfiguration& config)
    : GraphReportStage("LINES_BAR", config) {}

std::shared_ptr<cider::mathplot::IBasicPlot> LinesBarPlotReportStage::makePlot(
    const std::string&,
    const std::string& logDir,
    const std::string& logFileName) {
  return std::make_shared<cider::mathplot::LinesBarPlot>(logDir, logFileName);
}

std::function<void(cider::mathplot::IBasicPlot* plot,
                   const std::string& methodName,
                   const std::string& libName,
                   const cider::Cmd& cmd,
                   const Result& result)>
LinesBarPlotReportStage::createProcessor() {
  return [](cider::mathplot::IBasicPlot* plot, const std::string& methodName,
            const std::string& /*libName*/, const cider::Cmd&,
            const Result& result) {
    auto* logger = dynamic_cast<cider::mathplot::LinesBarPlot*>(plot);
    logger->log(result.testCaseName, methodName, result.oldActions.size(),
                result.newActions.size());
  };
}

}  // namespace pipelines
}  // namespace cider
