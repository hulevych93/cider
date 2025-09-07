// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "cfg-heatmap-plot-report-pipe.h"

#include "mathplot-log/output/comp-coverage-heatmap.h"

namespace cider {
namespace pipelines {

HeatmapPlotReportStage::HeatmapPlotReportStage(
    const ReportConfiguration& config)
    : GraphReportStage("HEATMAP", config) {}

std::shared_ptr<cider::mathplot::IBasicPlot> HeatmapPlotReportStage::makePlot(
    const std::string&,
    const std::string& logDir,
    const std::string& logFileName) {
  return std::make_shared<cider::mathplot::CoverageHeatmapPlot>(logDir,
                                                                logFileName);
}

std::function<void(cider::mathplot::IBasicPlot* plot,
                   const std::string& methodName,
                   const std::string& libName,
                   const cider::Cmd& cmd,
                   const Result& result)>
HeatmapPlotReportStage::createProcessor() {
  return [this](cider::mathplot::IBasicPlot* plot,
                const std::string& methodName, const std::string&,
                const cider::Cmd&, const Result& result) {
    auto* logger = dynamic_cast<cider::mathplot::CoverageHeatmapPlot*>(plot);
    logger->setRef(result.oldCfgReport.coveredTracks);

    logger->add(methodName, result.newCgfReport.coveredTracks);
  };
}

}  // namespace pipelines
}  // namespace cider
