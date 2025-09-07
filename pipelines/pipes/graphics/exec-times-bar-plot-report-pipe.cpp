// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "exec-times-bar-plot-report-pipe.h"

#include "mathplot-log/output/comp-exec-time-barplot.h"

#include "pipelines/metrics.h"

#include "pipelines/metrics.h"

#include <iostream>

namespace cider {
namespace pipelines {

ExecTimesBarPlotReportStage::ExecTimesBarPlotReportStage(
    const ReportConfiguration& config)
    : GraphReportStage("EXEC_TIMES_BAR", config),
      _filter1(50, 3.5),
      _filter2(50, 3.5) {}

std::shared_ptr<cider::mathplot::IBasicPlot>
ExecTimesBarPlotReportStage::makePlot(const std::string&,
                                      const std::string& logDir,
                                      const std::string& logFileName) {
  return std::make_shared<cider::mathplot::ExecTimesBarPlot>(logDir,
                                                             logFileName);
}

std::function<void(cider::mathplot::IBasicPlot* plot,
                   const std::string& methodName,
                   const std::string& libName,
                   const cider::Cmd& cmd,
                   const Result& result)>
ExecTimesBarPlotReportStage::createProcessor() {
  return [this](cider::mathplot::IBasicPlot* plot,
                const std::string& methodName, const std::string& libName,
                const cider::Cmd&, const Result& result) {
    auto* logger = dynamic_cast<cider::mathplot::ExecTimesBarPlot*>(plot);

    getCompression(methodName, libName, result,
                   [&](unsigned long /*covReachLen*/, double) {
                     if (_filter1.accept(methodName, result.testCaseName,
                                         result.oldExecutionTimeMcs) &&
                         _filter2.accept(methodName, result.testCaseName,
                                         result.newExecutionTimeMcs)) {
                       logger->log(methodName, result.oldExecutionTimeMcs,
                                   result.newExecutionTimeMcs);
                     } else {
                       std::cout << "SKIP: " << methodName << " "
                                 << result.testCaseName << " "
                                 << result.oldExecutionTimeMcs << " "
                                 << result.newExecutionTimeMcs << std::endl;
                     }
                   });
  };
}

}  // namespace pipelines
}  // namespace cider
