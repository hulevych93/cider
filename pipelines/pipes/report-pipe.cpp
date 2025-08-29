// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "report-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "pipelines/metrics.h"

#include "mathplot-log/output/comp-coverage-box-plot.h"
#include "mathplot-log/output/comp-coverage-grow-plot.h"
#include "mathplot-log/output/comp-coverage-heatmap.h"
#include "mathplot-log/output/comp-lines-barplot.h"
#include "mathplot-log/output/comp-time-barplot.h"

#include "mathplot-log/monitoring/q-learning-cov-ep-plot.h"
#include "mathplot-log/monitoring/q-learning-reward-loss-plot.h"

#include <iostream>

namespace cider {
namespace pipelines {

namespace {

bool ourMethod(const std::string& name) {
  static const std::vector<std::string> orderedMethods = {
      "QLEG1", "QLEG2", "QLEG3", "QLB1", "QLB2", "QLB3"};
  return std::find(orderedMethods.cbegin(), orderedMethods.cend(), name) !=
         orderedMethods.cend();
}

template <typename F>
void processBest(const std::string& methodName,
                 const std::vector<Result>& results,
                 int max,
                 F&& func) {
  std::vector<Result> bestResults = results;

  if (ourMethod(methodName)) {
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
void processBatchBest(const std::string& methodName,
                      const std::vector<Result>& results,
                      int max,
                      F&& func) {
  std::vector<Result> bestResults = results;

  if (ourMethod(methodName)) {
    std::sort(bestResults.begin(), bestResults.end(),
              [](const auto& l, const auto& r) {
                return l.newReport.branchCov.covered >
                       r.newReport.branchCov.covered;
              });
  }

  if (max < bestResults.size()) {
    bestResults.erase(bestResults.begin() + max, bestResults.end());
  }

  func(methodName, bestResults);
}

constexpr int DataSize = 70;  // bitmap++

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

StepperReportStage::StepperReportStage(const ReportConfiguration& config)
    : _config(config) {}

bool StepperReportStage::process(const std::string& metadata,
                                 const std::string& libName,
                                 const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

  std::string graphTitle =
      "STEP_" + libName + "_" + std::to_string(DataSize) + "_";
  for (const auto& entry : _config) {
    graphTitle += entry;
    graphTitle += "_";
  }

  auto logger = std::make_shared<cider::mathplot::StepperComparativeLogger>(
      outPath.string(), graphTitle,
      cider::mathplot::StepperComparativeLogger::PlotType::BrCov);

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

      processBest(name, res.entries, DataSize, handleResult);
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

  std::string graphTitle = "CBOX" + libName + "_";
  for (const auto& entry : _config) {
    graphTitle += entry;
    graphTitle += "_";
  }

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

    processBest(name, res.entries, DataSize, handleResult);
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

  std::string graphTitle = "LINES_BAR_" + std::to_string(DataSize) + "_";
  for (const auto& entry : _config) {
    graphTitle += entry;
    graphTitle += "_";
  }

  auto logger = std::make_shared<cider::mathplot::LinesBarPlot>(
      outPath.string(), graphTitle);

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

      processBest(name, res.entries, DataSize, handleResult);
    }
  }

  logger->plot();

  return true;
}

TimesBarPlotReportStage::TimesBarPlotReportStage(
    const ReportConfiguration& config)
    : _config(config) {}

bool TimesBarPlotReportStage::process(const std::string& metadata,
                                      const std::string&,
                                      const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

  std::string graphTitle = "TIMES_BAR_" + std::to_string(DataSize) + "_";
  for (const auto& entry : _config) {
    graphTitle += entry;
    graphTitle += "_";
  }

  auto logger = std::make_shared<cider::mathplot::TimesBarPlot>(
      outPath.string(), graphTitle);

  FilterManager filterManager(50, 3.5);

  const auto handleResult = [&](const std::string& methodName,
                                const Result& result) {
    // if (filterManager.accept(methodName, result.testCaseName,
    // result.timeElapsedMcs)) {
    logger->log(methodName, result.timeElapsedMcs);
    // } else {
    // за бажанням: залогувати, що значення відкинуто як викид
    //  std::cout << "SKIP: " << methodName << " " << result.testCaseName << " "
    //  << result.timeElapsedMcs << std::endl;
    // }
  };

  const auto& results = getResults();

  for (const auto& methodConfig : _config) {
    const auto it = results.find(methodConfig);
    if (it == results.end()) {
      std::cout << "Warning method not simlated: " << methodConfig << std::endl;
      continue;
    }
    const auto& name = it->first;
    const auto& res = it->second;

    processBest(name, res.entries, DataSize, handleResult);
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

  std::string graphTitle = "HEATMAP_COVERAGE_" + std::to_string(DataSize) + "_";
  for (const auto& entry : _config) {
    graphTitle += entry;
    graphTitle += "_";
  }

  auto logger = std::make_shared<cider::mathplot::CoverageHeatmapPlot>(
      outPath.string(), graphTitle);

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

    processBest(name, res.entries, DataSize, handleResult);
  }

  logger->plot();
  return true;
}

static inline void csvEsc(std::ostream& os, const std::string& s) {
  const bool needQuotes = s.find_first_of(";\"\n\r\t") != std::string::npos;
  if (!needQuotes) {
    os << s;
    return;
  }
  os << '"';
  for (char c : s)
    os << (c == '"' ? "\"\"" : std::string(1, c));
  os << '"';
}

EfficencyReportStage::EfficencyReportStage(const ReportConfiguration& config)
    : _config(config) {}

bool EfficencyReportStage::process(const std::string& metadata,
                                   const std::string&,
                                   const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

  std::error_code ec;
  std::filesystem::create_directories(outPath, ec);
  if (ec) {
    std::cerr << "Cannot create dir: " << outPath << " : " << ec.message()
              << std::endl;
    return false;
  }

  const auto& testOrLibName = getInput().testOrLibName;

  outPath /= "efficency_table_" + testOrLibName + ".csv";
  const bool writeHeader = !std::filesystem::exists(outPath);

  std::ofstream out(outPath, std::ios::app | std::ios::binary);
  if (!out) {
    std::cerr << "Cannot write to: " << outPath << std::endl;
    return false;
  }
  out.imbue(std::locale::classic());

  if (writeHeader) {
    out << "\xEF\xBB\xBF";  // UTF-8 BOM
    out << "sep=;\n";

    out << "Test Case;Method;"
        << "Old Coverage (%);New Coverage (%);Delta Coverage;"
        << "Old CFG (%);New CFG (%);Delta CFG;"
        << "Old Length;New Length;Cov Reach Len; Compression;"
        << "Execution Time (ms);"
        << "Total Processing (ms);Sessions;Coverage Improved;"
        << "Coverage Improvement Rate (%)\n";
  }

  auto fmt_pm = [](double avg, double stddev) {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << std::fixed << std::setprecision(2) << avg << " +/- " << stddev;
    return oss.str();
  };

  auto fmt = [](double v) {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << std::fixed << std::setprecision(2) << v << "'";
    return oss.str();
  };

  auto csvEsc = [](std::ostream& os, const std::string& s) {
    const bool needQuotes = s.find_first_of(";\"\n\r\t") != std::string::npos;
    if (!needQuotes) {
      os << s;
      return;
    }
    os << '"';
    for (char c : s)
      os << (c == '"' ? "\"\"" : std::string(1, c));
    os << '"';
  };

  const auto& results = getResults();
  for (const auto& methodConfig : _config) {
    const auto it = results.find(methodConfig);
    if (it == results.end()) {
      std::cout << "Warning method not simulated: " << methodConfig
                << std::endl;
      continue;
    }

    const auto& method = it->first;
    const auto& pack = it->second;

    const auto handleResults = [&](const std::string& methodName,
                                   const std::vector<Result>& results) {
      Metrics m = computeMetrics(results);

      unsigned long sessions = results.size();
      unsigned long covHit = 0;
      unsigned long totalProcMcs = 0;

      for (const auto& r : results) {
        totalProcMcs += r.timeElapsedMcs;
        if (r.newReport.branchCov.covered >= r.oldReport.branchCov.covered) {
          covHit++;
        }
      }

      const double totalProcMs = static_cast<double>(totalProcMcs) / 1000.0;
      const double covRate =
          sessions ? (100.0 * static_cast<double>(covHit) / sessions) : 0.0;

      csvEsc(out, testOrLibName);
      out << ';';
      csvEsc(out, methodName);
      out << ';';

      out << fmt_pm(m.avgOldCov, m.stdOldCov) << ';'
          << fmt_pm(m.avgNewCov, m.stdNewCov) << ';' << fmt(m.covDelta) << ';'

          << fmt_pm(m.avgOldCfg, m.stdOldCfg) << ';'
          << fmt_pm(m.avgNewCfg, m.stdNewCfg) << ';' << fmt(m.cfgDelta) << ';'

          << fmt_pm(m.avgOldLen, m.stdOldLen) << ';'
          << fmt_pm(m.avgNewLen, m.stdNewLen) << ';'
          << fmt_pm(m.avgCovReachLen, m.stdCovReachLen) << ';'
          << fmt(m.compression) << ';'

          << fmt_pm(m.avgTime, m.stdTime) << ';'

          << fmt(totalProcMs) << ';' << sessions << ';' << covHit << ';'
          << fmt(covRate) << '\n';
    };

    // filter entries by test name
    std::vector<Result> filtered;
    filtered.reserve(pack.entries.size());
    for (const auto& e : pack.entries) {
      if (e.testCaseName == testOrLibName)
        filtered.emplace_back(e);
    }
    if (filtered.empty())
      continue;

    processBatchBest(method, filtered, DataSize, handleResults);
  }

  out.flush();
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

ShowResultsStage::ShowResultsStage(const ReportConfiguration& config)
    : _config(config) {}

bool ShowResultsStage::process(const std::string&,
                               const std::string&,
                               const cider::Cmd&) {
  if (_config.empty()) {
    std::cout << "Empty config error." << std::endl;
    return false;
  }

  for (const auto& methodConfig : _config) {
    printResultsSummary(methodConfig, getResults());
  }

  return true;
}

bool ProcessDataStage::process(const std::string&,
                               const std::string&,
                               const cider::Cmd&) {
  return true;
}

}  // namespace pipelines
}  // namespace cider
