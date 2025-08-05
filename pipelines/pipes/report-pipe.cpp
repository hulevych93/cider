// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "report-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "pipelines/metrics.h"

#ifdef ENABLE_MATHPLOT
#include "mathplot-log/output/comp-coverage-box-plot.h"
#include "mathplot-log/output/comp-coverage-grow-plot.h"
#include "mathplot-log/output/comp-coverage-heatmap.h"
#include "mathplot-log/output/comp-lines-barplot.h"
#endif

#include <iostream>

namespace cider {
namespace pipelines {

namespace {

template <typename F>
void processBest(const std::string& methodName,
                 const std::vector<Result>& results,
                 int max,
                 F&& func) {
  std::vector<Result> bestResults = results;

  if (methodName != "RAND") {
    std::sort(bestResults.begin(), bestResults.end(),
              [](const auto& l, const auto& r) {
                return l.newReport.branchCov.covered >
                       r.newReport.branchCov.covered;
              });
  }

  int i = 0;
  for (const auto& rs : bestResults) {
    func(methodName, rs);
    ++i;
    if (i > max) {
      break;
    }
  }
}

template <typename F>
void processBestBatch(const std::string& methodName,
                      const std::vector<Result>& results,
                      int max,
                      F&& func) {
  std::vector<Result> bestResults = results;

  std::sort(
      bestResults.begin(), bestResults.end(), [](const auto& l, const auto& r) {
        return l.newReport.branchCov.covered > r.newReport.branchCov.covered;
      });

  if (max > 0 && static_cast<size_t>(max) < bestResults.size()) {
    bestResults.resize(max);
  }

  func(methodName, bestResults);
}

}  // namespace

StepperReportStage::StepperReportStage(const ReportConfiguration& config)
    : _config(config) {}

bool StepperReportStage::process(const std::string& metadata,
                                 const std::string& libName,
                                 const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

  int dataSize = 1000;

  std::string graphTitle =
      "STEP_" + libName + "_" + std::to_string(dataSize) + "_";
  for (const auto& entry : _config) {
    graphTitle += entry;
    graphTitle += "_";
  }

#ifdef ENABLE_MATHPLOT
  auto logger = std::make_shared<cider::mathplot::StepperComparativeLogger>(
      outPath.string(), graphTitle,
      cider::mathplot::StepperComparativeLogger::PlotType::BrCov);
#endif

  if (!logger->load()) {
    const auto handleResult = [&](const std::string& methodName,
                                  const Result& result) {
      logger->next(methodName);

      cider::gcov_coverage::StepperCoverageMeasurment stepper{cmd,
                                                              libName.c_str()};

      stepper.setLogger(logger);

      stepper.measure(result.newActions);
    };

    const auto& input = getInput();

    Result origin;
    origin.newActions = input.actions;
    handleResult("Original", origin);

    const auto& results = getResults();

    if (_config.empty()) {
      std::cout << "Empty config error." << std::endl;
      return false;
    }

    for (const auto& methodConfig : _config) {
      const auto it = results.find(methodConfig);
      if (it == results.end()) {
        std::cout << "Warning method not simlated: " << methodConfig
                  << std::endl;
        continue;
      }
      const auto& name = it->first;
      const auto& res = it->second;

      processBest(name, res, dataSize, handleResult);
    }
  }

  logger->plot();

  return true;
}

BoxPlotReportStage::BoxPlotReportStage(const ReportConfiguration& config)
    : _config(config) {}

bool BoxPlotReportStage::process(const std::string& metadata,
                                 const std::string& libName,
                                 const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

  int dataSize = 150;

  std::string graphTitle =
      "CBOX" + libName + "_" + std::to_string(dataSize) + "_";
  for (const auto& entry : _config) {
    graphTitle += entry;
    graphTitle += "_";
  }

#ifdef ENABLE_MATHPLOT
  auto brCovLogger = std::make_shared<cider::mathplot::CoverageBoxPlot>(
      outPath.string(), graphTitle,
      cider::mathplot::CoverageBoxPlot::PlotType::BrCov);

  /*
  auto lnCovLogger = std::make_shared<cider::mathplot::CoverageBoxPlot>(
      outPath.string(), "ln_box_plot.png",
      cider::mathplot::CoverageBoxPlot::PlotType::LineCov); */

  auto compositeLogger =
      std::make_shared<cider::gcov_coverage::CompositeLogger>();
  compositeLogger->addLogger(brCovLogger);
  // compositeLogger->addLogger(lnCovLogger);
#endif

  std::string lastLabel;

  const auto handleResult = [&](const std::string& name, const Result& result) {
    if (lastLabel.empty() || lastLabel != name) {
      lastLabel = name;
      brCovLogger->next(name);
      // lnCovLogger->next(name);
    }

    gcov_coverage::RootReport report;
    report.report = result.newReport;
    brCovLogger->log(0U, report);
    // lnCovLogger->log(0U, report);
  };

  const auto& results = getResults();

  if (_config.empty()) {
    std::cout << "Empty config error." << std::endl;
    return false;
  }

  for (const auto& methodConfig : _config) {
    const auto it = results.find(methodConfig);
    if (it == results.end()) {
      std::cout << "Warning method not simlated: " << methodConfig << std::endl;
      continue;
    }
    const auto& name = it->first;
    const auto& res = it->second;

    processBest(name, res, dataSize, handleResult);
  }

  brCovLogger->plot();

  // lnCovLogger->plot();
  // lnCovLogger->save();

  return true;
}

LinesBarPlotReportStage::LinesBarPlotReportStage(
    const ReportConfiguration& config)
    : _config(config) {}

bool LinesBarPlotReportStage::process(const std::string&,
                                      const std::string&,
                                      const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);

  int dataSize = 1000;

  std::string graphTitle = "LINES_BAR_" + std::to_string(dataSize) + "_";
  for (const auto& entry : _config) {
    graphTitle += entry;
    graphTitle += "_";
  }

#ifdef ENABLE_MATHPLOT
  auto logger = std::make_shared<cider::mathplot::LinesBarPlot>(
      outPath.string(), graphTitle);
#endif

  if (!logger->load()) {
    const auto handleResult = [&](const std::string& methodName,
                                  const Result& result) {
      logger->log(result.testCaseName, methodName, result.oldActions.size(),
                  result.newActions.size());
    };

    const auto& results = getResults();

    for (const auto& methodConfig : _config) {
      const auto it = results.find(methodConfig);
      if (it == results.end()) {
        std::cout << "Warning method not simlated: " << methodConfig
                  << std::endl;
        continue;
      }
      const auto& name = it->first;
      const auto& res = it->second;

      processBest(name, res, dataSize, handleResult);
    }
  }

  logger->plot();

  return true;
}

HeatmapPlotReportStage::HeatmapPlotReportStage(
    const ReportConfiguration& config)
    : _config(config) {}

bool HeatmapPlotReportStage::process(const std::string& metadata,
                                     const std::string& libName,
                                     const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

  int dataSize = 100;

  std::string graphTitle = "HEATMAP_COVERAGE_" + std::to_string(dataSize) + "_";
  for (const auto& entry : _config) {
    graphTitle += entry;
    graphTitle += "_";
  }

#ifdef ENABLE_MATHPLOT
  auto logger = std::make_shared<cider::mathplot::CoverageHeatmapPlot>(
      outPath.string(), graphTitle);
#endif

  const auto handleResult = [&](const std::string& methodName,
                                const Result& result) {
    logger->setRef(result.oldCfgReport.coveredTracks);

    logger->add(methodName, result.newCgfReport.coveredTracks);
  };

  const auto& results = getResults();

  for (const auto& methodConfig : _config) {
    const auto it = results.find(methodConfig);
    if (it == results.end()) {
      std::cout << "Warning: method not simulated: " << methodConfig << "\n";
      continue;
    }

    const auto& name = it->first;
    const auto& res = it->second;

    processBest(name, res, dataSize, handleResult);
  }

  logger->plot();
  return true;
}

bool EfficencyReportStage::process(const std::string& metadata,
                                   const std::string&,
                                   const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  std::filesystem::create_directories(outPath);
  outPath /= "efficency_table.csv";

  std::ofstream out(outPath);
  if (!out) {
    std::cerr << "Cannot write to: " << outPath << std::endl;
    return false;
  }

  // Header with average ± stddev
  out << "Method;"
      << "OldCov(%);NewCov(%);DeltaCov;"
      << "OldCFG(%);NewCFG(%);DeltaCFG;"
      << "OldLen;NewLen;Compression;"
      << "Time(ms);EffScore\n";

  std::locale::global(std::locale("C"));

  const auto handleResult = [&](const std::string& method,
                                const std::vector<Result>& results) {
    Metrics m = computeMetrics(results);

    auto fmt = [](double avg, double stddev) {
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(2) << avg << " ± " << stddev;
      return oss.str();
    };

    out << method << ";" << fmt(m.avgOldCov, m.stdOldCov) << ";"
        << fmt(m.avgNewCov, m.stdNewCov) << ";" << fmt(m.covDelta, 0.0)
        << ";"  // stddev for delta is not reported
        << fmt(m.avgOldCfg, m.stdOldCfg) << ";" << fmt(m.avgNewCfg, m.stdNewCfg)
        << ";" << fmt(m.cfgDelta, 0.0) << ";" << fmt(m.avgOldLen, m.stdOldLen)
        << ";" << fmt(m.avgNewLen, m.stdNewLen) << ";"
        << fmt(m.compression, 0.0) << ";" << fmt(m.avgTime, m.stdTime) << ";"
        << fmt(m.effScore, 0.0) << "\n";
  };

  const auto& results = getResults();
  for (const auto& resIt : results) {
    const auto& name = resIt.first;
    const auto& res = resIt.second;

    processBestBatch(name, res, 150, handleResult);
  }

  return true;
}

RemoveDataStage::RemoveDataStage(const ReportConfiguration& config)
    : _config(config) {}

bool RemoveDataStage::process(const std::string&,
                              const std::string&,
                              const cider::Cmd&) {
  if (_config.empty()) {
    std::cout << "Empty config error." << std::endl;
    return false;
  }

  for (const auto& methodConfig : _config) {
    clearData(methodConfig);
  }

  return true;
}

bool ProcessDataStage::process(const std::string&,
                               const std::string&,
                               const cider::Cmd&) {
  auto& mutableData = getMutableResults();

  auto copys = getMutableResults();

  mutableData["QLEG2"] = copys["QLEG1"];

  return true;
}

}  // namespace pipelines
}  // namespace cider
