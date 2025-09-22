// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "eft-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "pipelines/metrics.h"

#include <tlog.h>

namespace cider {
namespace pipelines {

namespace {

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

void csvEsc(std::ostream& os, const std::string& s) {
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

}  // namespace

EfficencyReportStage::EfficencyReportStage(const ReportConfiguration& config)
    : _config(config) {}

bool EfficencyReportStage::process(const std::string& metadata,
                                   const std::string& libName,
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

    out << "Method;"
        << "Coverage (%);Delta Coverage;"
        << "CFG (%);Delta CFG;"
        << "Length;CovReach Len;"
        << "Compression;Std Compression;"
        << "Old Exec Time (ms);New Exec Time (ms);"
        << "Time Reduction (%);Std Time Reduction;"
        << "Total Processing (ms);"
        << "Retained/Total;Coverage Retention Rate (%);"
        << "J(TC)\n";
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

  const auto& results = getResults();
  for (const auto& methodConfig : _config) {
    const auto it = results.find(methodConfig);
    if (it == results.end()) {
      tlog_info << "Warning method not simulated: " << methodConfig
                << std::endl;
      continue;
    }

    const auto& method = it->first;
    const auto& pack = it->second;

    const auto handleResults = [&](const std::string& methodName,
                                   const std::vector<Result>& results) {
      Metrics m = computeMetrics(methodName, results);

      csvEsc(out, methodName);
      out << ';';

      out << fmt_pm(m.avgNewCov, m.stdNewCov) << ';' << fmt(m.covDelta) << ';'

          << fmt_pm(m.avgNewCfg, m.stdNewCfg) << ';' << fmt(m.cfgDelta) << ';'

          << fmt_pm(m.avgNewLen, m.stdNewLen) << ';'
          << fmt_pm(m.avgCovReachLen, m.stdCovReachLen) << ';'

          << fmt(m.compression) << ';' << fmt(m.stdCompression) << ';'

          << fmt_pm(m.avgOldTime, m.stdOldTime) << ';'
          << fmt_pm(m.avgNewTime, m.stdNewTime) << ';'
          << fmt(m.timeReduction * 100.0) << ';'
          << fmt(m.stdTimeReduction * 100.0) << ';'

          << fmt_pm(m.avgTotalTime, m.stdTotalTime) << ';'
          << fmt(m.retainedCount) << "/" << fmt(m.totalCount) << ';'
          << fmt(m.coverageRetentionRate) << ';'

          << fmt(m.jScore) << '\n';
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

    processBatchBest(method, filtered, getDataSize(libName), handleResults);
  }

  out.flush();
  return true;
}

}  // namespace pipelines
}  // namespace cider
