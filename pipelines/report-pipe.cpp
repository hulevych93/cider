// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "report-pipe.h"

#include "recorder/details/generator.h"

#include "q-learning/agent.h"
#include "q-learning/q-learning.h"
#include "q-learning/scenario.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#ifdef ENABLE_MATHPLOT
#include "mathplot-log/mathplot-log.h"
#endif

#include <iostream>

using namespace cider::qleaning;

namespace cider {
namespace pipelines {

bool StepperReportStage::process(const std::string& metadata,
                                 const std::string& libName,
                                 const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

#ifdef ENABLE_MATHPLOT
  auto logger =
      std::make_shared<cider::gcov_coverage::StepperComparativeLogger>(
          outPath.string(), "comparage.png", cider::PlotType::BrCov);
#endif

  auto generator = cider::recorder::makeLuaGenerator(libName);

  const auto handleResult = [&](const Result& result) {
    logger->next(result.methodName);

    cider::gcov_coverage::StepperCoverageMeasurment stepper{cmd,
                                                            libName.c_str()};

    stepper.setLogger(logger);

    stepper.measure(result.actions);

    const auto script =
        cider::recorder::generateScript(generator, result.actions, 99999U);

    std::ofstream output_file(outPath /
                              (libName + "_" + result.testOrLibName + ".lua"));
    output_file << script;
  };

  const auto& input = getInput();

  Result origin;
  origin.actions = input.actions;
  origin.methodName = "Original";
  handleResult(origin);

  auto getBest = [&](const std::string& name, int max) {
      Results bestResults;

      const auto& results = getResults();
      for (const auto& result : results) {
          if (result.methodName != name) {
              continue;
          }

          bestResults.emplace_back(result);
      }

      cider::gcov_coverage::CoverageMeasurment msr{cmd, libName.c_str()};
      std::sort(bestResults.begin(), bestResults.end(), [&msr](const auto& l, const auto& r) {
          const auto& lreport = msr.getReport(l.actions);
          const auto& rreport = msr.getReport(r.actions);
          if (lreport.has_value() && rreport.has_value()) {
              return lreport->report.branchCov.covered > rreport->report.branchCov.covered;
          }
          return false;
      });

      int i = 0;
      for(const auto& rs: bestResults) {
          handleResult(rs);
          ++i;
          if(i > max) {
              break;
          }
      }

  };

  getBest("QLG2", 2);
  getBest("QLB2", 10);
  getBest("RAND", 10);

  logger->plot();

  return true;
}

bool BoxPlotReportStage::process(const std::string& metadata,
                                 const std::string& libName,
                                 const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

#ifdef ENABLE_MATHPLOT
  auto brCovLogger = std::make_shared<cider::gcov_coverage::CoverageBoxPlot>(
      outPath.string(), "br_box_plot.png", PlotType::BrCov);
  auto lnCovLogger = std::make_shared<cider::gcov_coverage::CoverageBoxPlot>(
      outPath.string(), "ln_box_plot.png", PlotType::LineCov);

  auto compositeLogger =
      std::make_shared<cider::gcov_coverage::CompositeLogger>();
  compositeLogger->addLogger(brCovLogger);
  compositeLogger->addLogger(lnCovLogger);
#endif

  std::string lastLabel;

  const auto handleResult = [&](const Result& result) {
    if (lastLabel.empty() || lastLabel != result.methodName) {
      lastLabel = result.methodName;
      brCovLogger->next(result.methodName);
      lnCovLogger->next(result.methodName);
    }

    cider::gcov_coverage::CoverageMeasurment msr{cmd, libName.c_str()};
    const auto& report = msr.getReport(result.actions);

    if (report.has_value() && report->report.branchCov.covered != 0) {


    brCovLogger->log(0U, report.value());
    lnCovLogger->log(0U, report.value());

        }
  };

 const auto& results = getResults();
  for (const auto& result : results) {
    handleResult(result);
  }

  brCovLogger->plot();
  brCovLogger->save();

  lnCovLogger->plot();
  lnCovLogger->save();

  return true;
}

bool CovBarPlotReportStage::process(const std::string&,
                                    const std::string&,
                                    const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.commonResultsDir);
  outPath /= "bar_plot";

#ifdef ENABLE_MATHPLOT
  auto logger = std::make_shared<cider::gcov_coverage::CoverageBarPlot>(
      outPath.string(), "dataset_bar_plot.png");
#endif

  std::string lastLabel;

  const auto handleResult = [&](const BriefResult& result) {
    if (lastLabel.empty() || lastLabel != result.testOrLibName) {
      lastLabel = result.testOrLibName;
      logger->next(result.testOrLibName);
    }

    gcov_coverage::RootReport report;
    report.report = result.report;
    logger->log(0U, report);
  };

  const auto& results = getBriefResults();
  for (const auto& result : results) {
    handleResult(result);
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
  auto logger = std::make_shared<cider::LinesBarPlot>(outPath.string(),
                                                      "lines_bar_plot.png");
#endif

  std::string lastLabel;

  const auto handleResult = [&](const BriefResult& result) {
    if (lastLabel.empty() || lastLabel != result.testOrLibName) {
      lastLabel = result.testOrLibName;
      logger->init(result.testOrLibName, result.oldLines);
    }

    logger->log(result.testOrLibName, result.methodName, result.newLines);
  };

  const auto& results = getBriefResults();
  for (const auto& result : results) {
    std::cout << result << std::endl;
    handleResult(result);
  }

  logger->plot();
  logger->save();

  return true;
}

}  // namespace pipelines
}  // namespace cider
