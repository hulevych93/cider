// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "graph-report-pipe.h"

#include "mathplot-log/monitoring/q-learning-cov-ep-plot.h"
#include "mathplot-log/monitoring/q-learning-reward-loss-plot.h"

#include <iostream>

namespace cider {
namespace pipelines {

bool QLearningReportStage::process(const std::string& metadata,
                                   const std::string&,
                                   const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

  mathplot::QLearningRewardLogger rwLogger(outPath, "reward.png");
  mathplot::QLearningLossLogger lossLogger(outPath, "loss.png");
  mathplot::CovQLearningResultsMathplotLogger covLogger(outPath, "cov.png");

  rwLogger.load();
  lossLogger.load();
  covLogger.load();

  return true;
}

GraphReportStage::GraphReportStage(const std::string& title,
                                   const ReportConfiguration& config)
    : _title(title), _config(config) {}

bool GraphReportStage::process(const std::string& metadata,
                               const std::string& libName,
                               const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

  std::string graphTitle =
      _title + "_" + libName + "_" + std::to_string(getDataSize(libName)) + "_";
  for (const auto& entry : _config) {
    graphTitle += entry;
    graphTitle += "_";
  }

  auto logger = makePlot(libName, outPath.string(), graphTitle);

  if (!logger->load()) {
    logger->setOrder(_config);
    const auto handleResult = createProcessor();

    const auto& results = getResults();

    for (const auto& methodConfig : _config) {
      const auto it = results.find(methodConfig);
      if (it == results.end()) {
        std::cout << "Warning: method not simulated: " << methodConfig << "\n";
        continue;
      }

      const auto& name = it->first;
      const auto& res = it->second;

      processBest(logger.get(), name, libName, cmd, res.entries,
                  getDataSize(libName), handleResult);
    }
  }

  logger->plot();
  return true;
}

}  // namespace pipelines
}  // namespace cider
