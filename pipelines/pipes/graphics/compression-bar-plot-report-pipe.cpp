// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "compression-bar-plot-report-pipe.h"

#include "mathplot-log/output/comp-compression-barplot.h"

#include "pipelines/metrics.h"

namespace cider {
namespace pipelines {

CompressionBarPlotReportStage::CompressionBarPlotReportStage(
    const ReportConfiguration& config)
    : GraphReportStage("COMPRESSION_BAR", config) {}

std::shared_ptr<cider::mathplot::IBasicPlot>
CompressionBarPlotReportStage::makePlot(const std::string&,
                                        const std::string& logDir,
                                        const std::string& logFileName) {
  return std::make_shared<cider::mathplot::CompressionBarPlot>(logDir,
                                                               logFileName);
}

std::function<void(cider::mathplot::IBasicPlot* plot,
                   const std::string& methodName,
                   const std::string& libName,
                   const cider::Cmd& cmd,
                   const Result& result)>
CompressionBarPlotReportStage::createProcessor() {
  return [this](cider::mathplot::IBasicPlot* plot,
                const std::string& methodName, const std::string& libName,
                const cider::Cmd&, const Result& result) {
    auto* logger = dynamic_cast<cider::mathplot::CompressionBarPlot*>(plot);
    getCompression(methodName, libName, result,
                   [&](unsigned long /*covReachLen*/, double coeff) {
                     logger->log(methodName, coeff);
                   });
  };
}

}  // namespace pipelines
}  // namespace cider
