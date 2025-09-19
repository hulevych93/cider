// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "cov-grow-plot-report-pipe.h"

#include "coverage/gcov_measurer.h"

#include "mathplot-log/output/comp-coverage-grow-plot.h"

#include "pipelines/metrics.h"

namespace cider {
namespace pipelines {

StepperReportStage::StepperReportStage(const ReportConfiguration& config)
    : GraphReportStage("STEP", config) {}

std::shared_ptr<cider::mathplot::IBasicPlot> StepperReportStage::makePlot(
    const std::string& libName,
    const std::string& logDir,
    const std::string& logFileName) {
  return std::make_shared<cider::mathplot::StepperComparativePlot>(
      libName, logDir, logFileName,
      cider::mathplot::StepperComparativePlot::PlotType::BrCov);
}

std::function<void(cider::mathplot::IBasicPlot* plot,
                   const std::string& methodName,
                   const std::string& libName,
                   const cider::Cmd& cmd,
                   const Result& result)>
StepperReportStage::createProcessor() {
  return [](cider::mathplot::IBasicPlot* plot, const std::string& methodName,
            const std::string& libName, const cider::Cmd& cmd,
            const Result& result) {
    auto* logger = dynamic_cast<cider::mathplot::StepperComparativePlot*>(plot);

    getCompression(
        methodName, libName, result, [&](unsigned long covReachLen, double) {
          logger->setOriginalCov(getOldCov(libName, result.oldReport));

          logger->next(methodName);
          cider::gcov_coverage::StepperCoverageMeasurment stepper{
              cmd, libName.c_str()};
          stepper.setLogger(logger);
          stepper.measure(result.newActions, getCoverageGrowStep(libName),
                          covReachLen);
        });
  };
}

}  // namespace pipelines
}  // namespace cider
