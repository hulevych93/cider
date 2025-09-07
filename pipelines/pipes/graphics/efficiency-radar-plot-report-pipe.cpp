// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "efficiency-radar-plot-report-pipe.h"

#include "mathplot-log/output/comp-efficiency-radarplot.h"

#include "pipelines/metrics.h"

#include <iostream>

namespace cider {
namespace pipelines {

EfficiencyRadarPlotReportStage::EfficiencyRadarPlotReportStage(
    const ReportConfiguration& config)
    : GraphReportStage("EFFICIENCY_RADAR", config),
      _filter1(50, 3.5),
      _filter2(50, 3.5),
      _filter(50, 3.5) {}

std::shared_ptr<cider::mathplot::IBasicPlot>
EfficiencyRadarPlotReportStage::makePlot(const std::string&,
                                         const std::string& logDir,
                                         const std::string& logFileName) {
  return std::make_shared<cider::mathplot::EfficiencyRadarPlot>(logDir,
                                                                logFileName);
}

std::function<void(cider::mathplot::IBasicPlot* plot,
                   const std::string& methodName,
                   const std::string& libName,
                   const cider::Cmd& cmd,
                   const Result& result)>
EfficiencyRadarPlotReportStage::createProcessor() {
  return [this](cider::mathplot::IBasicPlot* plot,
                const std::string& methodName, const std::string&,
                const cider::Cmd&, const Result& r) {
    auto* logger = dynamic_cast<cider::mathplot::EfficiencyRadarPlot*>(plot);

    bool retained = false;
    getCompression(
        methodName, r.testCaseName, r, [&](unsigned long, double coeff) {
          retained = true;

          logger->logCompression(methodName, coeff);

          if (r.oldExecutionTimeMcs > 0 &&
              r.oldExecutionTimeMcs > r.newExecutionTimeMcs) {
            const double tr =
                (double)(r.oldExecutionTimeMcs - r.newExecutionTimeMcs) /
                (double)r.oldExecutionTimeMcs;

            logger->logTimeReduction(methodName, tr);
          }
        });

    if (_filter1.accept("method", r.testCaseName, r.oldExecutionTimeMcs) &&
        _filter2.accept("method", r.testCaseName, r.newExecutionTimeMcs)) {
      logger->logProcessingTime(methodName, (double)r.newExecutionTimeMcs /
                                                (double)r.oldExecutionTimeMcs);
    }

    if (_filter.accept("method", r.testCaseName, r.timeElapsedMcs)) {
      logger->logProcessingTime(methodName, r.timeElapsedMcs);
    }

    logger->logCoverage(methodName, r.newReport.branchCov.percent);
  };
}

}  // namespace pipelines
}  // namespace cider
