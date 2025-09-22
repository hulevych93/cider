// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "times-bar-plot-report-pipe.h"

#include "mathplot-log/output/comp-time-barplot.h"

#include <tlog.h>

namespace cider {
namespace pipelines {

TimesBarPlotReportStage::TimesBarPlotReportStage(
    const ReportConfiguration& config)
    : GraphReportStage("TIMES_BAR", config), _filter(50, 3.5) {}

std::shared_ptr<cider::mathplot::IBasicPlot> TimesBarPlotReportStage::makePlot(
    const std::string&,
    const std::string& logDir,
    const std::string& logFileName) {
  return std::make_shared<cider::mathplot::TimesBarPlot>(logDir, logFileName);
}

std::function<void(cider::mathplot::IBasicPlot* plot,
                   const std::string& methodName,
                   const std::string& libName,
                   const cider::Cmd& cmd,
                   const Result& result)>
TimesBarPlotReportStage::createProcessor() {
  return [this](cider::mathplot::IBasicPlot* plot,
                const std::string& methodName, const std::string& /*libName*/,
                const cider::Cmd&, const Result& result) {
    auto* logger = dynamic_cast<cider::mathplot::TimesBarPlot*>(plot);
    if (_filter.accept(methodName, result.testCaseName,
                       result.timeElapsedMcs)) {
      logger->log(methodName, result.timeElapsedMcs);
    } else {
      tlog_info << "SKIP: " << methodName << " " << result.testCaseName << " "
                << result.timeElapsedMcs << std::endl;
    }
  };
}

}  // namespace pipelines
}  // namespace cider
