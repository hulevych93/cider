// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "report-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#ifdef ENABLE_MATHPLOT
#include "mathplot-log/output/comp-coverage-box-plot.h"
#include "mathplot-log/output/comp-coverage-grow-plot.h"
#include "mathplot-log/output/comp-dataset-coverage-bar-plot.h"
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

  std::sort(
      bestResults.begin(), bestResults.end(), [](const auto& l, const auto& r) {
        return l.newReport.branchCov.covered > r.newReport.branchCov.covered;
      });

  int i = 0;
  for (const auto& rs : bestResults) {
    func(methodName, rs);
    ++i;
    if (i > max) {
      break;
    }
  }
}

}  // namespace

bool StepperReportStage::process(const std::string& metadata,
                                 const std::string& libName,
                                 const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

#ifdef ENABLE_MATHPLOT
  auto logger = std::make_shared<cider::mathplot::StepperComparativeLogger>(
      outPath.string(), "comparage.png",
      cider::mathplot::StepperComparativeLogger::PlotType::BrCov);
#endif

  auto generator = cider::recorder::makeLuaGenerator(libName);

  const auto handleResult = [&](const std::string& methodName,
                                const Result& result) {
    logger->next(methodName);

    cider::gcov_coverage::StepperCoverageMeasurment stepper{cmd,
                                                            libName.c_str()};

    stepper.setLogger(logger);

    stepper.measure(result.newActions);

    const auto script =
        cider::recorder::generateScript(generator, result.newActions, 99999U);

    std::ofstream output_file(outPath /
                              (libName + "_" + result.testCaseName + ".lua"));
    output_file << script;
  };

  const auto& input = getInput();

  Result origin;
  origin.newActions = input.actions;
  handleResult("Original", origin);

  const auto& results = getResults();
  for (const auto& resIt : results) {
    const auto& name = resIt.first;
    const auto& res = resIt.second;

    processBest(name, res, 10, handleResult);
  }

  logger->plot();

  return true;
}

bool BoxPlotReportStage::process(const std::string& metadata,
                                 const std::string& libName,
                                 const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

#ifdef ENABLE_MATHPLOT
  auto brCovLogger = std::make_shared<cider::mathplot::CoverageBoxPlot>(
      outPath.string(), "br_box_plot.png",
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
  for (const auto& resIt : results) {
    const auto& name = resIt.first;
    const auto& res = resIt.second;

    processBest(name, res, 30, handleResult);
  }

  brCovLogger->plot();
  brCovLogger->save();

  // lnCovLogger->plot();
  // lnCovLogger->save();

  return true;
}

bool CovBarPlotReportStage::process(const std::string&,
                                    const std::string&,
                                    const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.commonResultsDir);
  outPath /= "bar_plot";

#ifdef ENABLE_MATHPLOT
  auto logger = std::make_shared<cider::mathplot::CoverageBarPlot>(
      outPath.string(), "dataset_bar_plot.png");
#endif

  std::string lastLabel;

  const auto handleResult = [&](const std::string& name, const Result& result) {
    if (lastLabel.empty() || lastLabel != name) {
      lastLabel = name;
      logger->next(name);
    }

    gcov_coverage::RootReport report;
    report.report = result.newReport;
    logger->log(0U, report);
  };

  const auto& results = getResults();
  for (const auto& resIt : results) {
    const auto& name = resIt.first;
    const auto& res = resIt.second;

    processBest(name, res, 10, handleResult);
  }

  logger->plot();
  logger->save();

  return true;
}

bool LinesBarPlotReportStage::process(const std::string& metadata,
                                      const std::string&,
                                      const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

#ifdef ENABLE_MATHPLOT
  auto logger = std::make_shared<cider::mathplot::LinesBarPlot>(
      outPath.string(), "lines_bar_plot.png");
#endif

  std::string lastLabel;

  const auto handleResult = [&](const std::string& name, const Result& result) {
    if (lastLabel.empty() || lastLabel != name) {
      lastLabel = name;
      logger->init(name, result.oldActions.size());
    }

    logger->log(result.testCaseName, name, result.newActions.size());
  };

  const auto& results = getResults();
  for (const auto& resIt : results) {
    const auto& name = resIt.first;
    const auto& res = resIt.second;

    processBest(name, res, 10, handleResult);
  }

  logger->plot();
  logger->save();

  return true;
}

}  // namespace pipelines
}  // namespace cider
