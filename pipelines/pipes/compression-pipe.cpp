// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "compression-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "pipelines/metrics.h"

#include <tlog.h>
#include <thread>

namespace cider {
namespace pipelines {

ResultsCompressionStage::ResultsCompressionStage(
    const ReportConfiguration& config)
    : _config(config) {}

bool ResultsCompressionStage::process(const std::string&,
                                      const std::string& libName,
                                      const cider::Cmd& cmd) {
  if (_config.empty()) {
    tlog_info << "Empty config error." << std::endl;
    return false;
  }

  int i = 0;
  auto results = getResults();
  for (const auto& methodConfig : _config) {
    auto it = results.find(methodConfig);
    if (it == results.end()) {
      tlog_info << "Warning method not simulated: " << methodConfig
                << std::endl;
      continue;
    }

    auto& method = it->first;
    auto& pack = it->second;

    i++;

    tlog_info << "  [METHOD] " << method
              << " | entries: " << pack.entries.size() << std::endl;

    int j = 0;
    for (auto& r : pack.entries) {
      j++;
      auto reached = computeCoverageReachedLength(r.oldActions, r.newActions,
                                                  libName, cmd);
      tlog_info << "[" << i << "," << results.size() << "][" << j << ","
                << pack.entries.size() << "]" << std::endl;

      size_t covLen = 0;
      if (reached.has_value()) {
        covLen = *reached;
        r.coverageReachedLength = *reached;
        tlog_info << "[OK] " << r.testCaseName
                  << " coverage reached at length = " << *reached << "\n";
      } else {
        covLen = r.newActions.size();
        r.coverageReachedLength = r.newActions.size();
        tlog_info << "[FAIL] " << r.testCaseName
                  << " never reached old coverage, "
                  << "fallback = full length "
                  << r.coverageReachedLength.value() << "\n";
      }

      if (covLen < r.newActions.size()) {
        auto trimmed = r.newActions;
        trimmed.erase(trimmed.begin() + covLen, trimmed.end());

        try {
          gcov_coverage::CoverageMeasurment measurer(cmd, libName.c_str());
          auto covVal = measurer.getReport(trimmed);
          if (!covVal.has_value()) {
            tlog_info << "[ERROR] Trigmmed coverage not available\n";
            continue;
          }

          auto covOld = measurer.getReport(r.newActions);
          if (!covOld.has_value()) {
            tlog_info << "[ERROR] Old coverage not available\n";
            continue;
          }

          if (covVal->report.branchCov.percent >=
              covOld->report.branchCov.percent * 0.999) {
            r.newActions = std::move(trimmed);
            tlog_info << "[TRIM-OK] " << r.testCaseName
                      << " validated and truncated to " << r.newActions.size()
                      << " actions\n";
          } else {
            tlog_info << "[TRIM-REJECTED] " << r.testCaseName
                      << " coverage drop detected ("
                      << covVal->report.branchCov.percent << " < "
                      << covOld->report.branchCov.percent << ")\n";
          }
        } catch (const std::exception& e) {
          tlog_info << "[TRIM-ERROR] " << r.testCaseName
                    << " failed validation: " << e.what() << "\n";
        }
      }
    }
  }

  replaceResults(results);
  return true;
}

}  // namespace pipelines
}  // namespace cider
