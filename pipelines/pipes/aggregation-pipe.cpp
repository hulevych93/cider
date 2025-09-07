// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "aggregation-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "pipelines/metrics.h"

#include <iostream>
#include <thread>

namespace cider {
namespace pipelines {

namespace {

std::unordered_map<std::string, std::vector<std::vector<Result>>>
groupIntoTestSets(const Results& results, const std::string& libName) {
  std::unordered_map<std::string, std::vector<std::vector<Result>>> methodSets;

  std::cout << "[INFO] Start grouping results into test sets..." << std::endl;
  for (const auto& it : results) {
    const auto& methodName = it.first;
    const auto& methodResults = it.second;

    std::cout << "  [METHOD] " << methodName
              << " | total entries: " << methodResults.entries.size()
              << std::endl;

    bool allAggregated =
        std::all_of(methodResults.entries.begin(), methodResults.entries.end(),
                    [&](const Result& r) { return r.testCaseName == libName; });

    if (allAggregated) {
      std::cout << "    [INFO] Detected pre-aggregated results for "
                << methodName << ", passing through as-is" << std::endl;
      std::vector<std::vector<Result>> sets;
      for (auto& r : methodResults.entries) {
        sets.push_back({r});
      }
      methodSets[methodName] = std::move(sets);
      continue;
    }

    std::map<std::string, std::vector<Result>> grouped;
    for (const auto& r : methodResults.entries) {
      grouped[r.testCaseName].push_back(r);
    }
    std::cout << "    grouped into " << grouped.size() << " test cases\n";

    size_t sessions = 0;
    for (const auto& it : grouped) {
      const auto& name = it.first;
      const auto& vec = it.second;

      sessions = std::max(sessions, vec.size());
    }
    std::cout << "    sessions detected: " << sessions << std::endl;

    std::vector<std::vector<Result>> sets(sessions);

    for (auto& itGr : grouped) {
      const auto& tcName = itGr.first;
      const auto& vec = itGr.second;

      std::cout << "      [TC] " << tcName << " | entries: " << vec.size()
                << std::endl;
      for (size_t i = 0; i < vec.size(); ++i) {
        sets[i].push_back(vec[i]);
      }
    }

    methodSets[methodName] = std::move(sets);
  }

  std::cout << "[INFO] Grouping finished" << std::endl;
  return methodSets;
}

std::optional<Result> aggregateSession(
    const std::vector<recorder::Action>& oldActions,
    const std::string& libName,
    const cider::Cmd& cmd,
    const std::vector<Result>& session,
    bool recheck = false) {
  Result aggregated;
  aggregated.testCaseName = libName;
  std::cout << "[INFO] Aggregating session with " << session.size()
            << " results..." << std::endl;

  aggregated.oldActions = oldActions;

  cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd, libName.c_str()};
  cider::cfg_coverage::CoverageMeasurment cfg_measurer{cmd, libName.c_str()};

  if (recheck) {
    for (const auto& r : session) {
      std::vector<recorder::Action> trialNewActions = aggregated.newActions;
      trialNewActions.insert(trialNewActions.end(), r.newActions.begin(),
                             r.newActions.end());

      auto trialGcov = gcov_measurer.getReport(trialNewActions);
      auto trialCfg = cfg_measurer.getReport(trialNewActions);

      if (!trialGcov.has_value() || !trialCfg.has_value()) {
        std::cout << "  [WARN] Skipping " << r.testCaseName
                  << " — invalid coverage report after adding actions\n";
        continue;
      }

      aggregated.newActions = std::move(trialNewActions);
      aggregated.timeElapsedMcs += r.timeElapsedMcs;
    }
  } else {
    for (const auto& r : session) {
      aggregated.newActions.insert(aggregated.newActions.end(),
                                   r.newActions.begin(), r.newActions.end());

      aggregated.timeElapsedMcs += r.timeElapsedMcs;
    }
  }

  std::cout << "  total time (mcs): " << aggregated.timeElapsedMcs
            << " | oldActions: " << aggregated.oldActions.size()
            << " | newActions: " << aggregated.newActions.size() << std::endl;

  // old coverage reports
  static cider::gcov_coverage::ReportOpt oldGcovReport;
  if (!oldGcovReport.has_value()) {
    oldGcovReport = gcov_measurer.getReport(aggregated.oldActions);
  }
  if (oldGcovReport.has_value()) {
    aggregated.oldReport = oldGcovReport->report;
  }

  static cider::cfg_coverage::CfgCoverageOpt oldCfgReport;
  if (!oldCfgReport.has_value()) {
    oldCfgReport = cfg_measurer.getReport(aggregated.oldActions);
  }
  if (oldCfgReport.has_value()) {
    aggregated.oldCfgReport = oldCfgReport.value();
    aggregated.oldExecutionTimeMcs = oldCfgReport->meassureTimeMcs;
  }

  // final coverage for aggregated newActions
  auto newGcovReport = gcov_measurer.getReport(aggregated.newActions);
  auto newCfgReport = cfg_measurer.getReport(aggregated.newActions);

  if (newGcovReport.has_value() && newCfgReport.has_value()) {
    aggregated.newReport = newGcovReport->report;
    aggregated.newCgfReport = newCfgReport.value();
    aggregated.newExecutionTimeMcs = newCfgReport->meassureTimeMcs;
    std::cout << "  Final coverage reports loaded" << std::endl;
  } else {
    std::cout << "  [ERROR] Final coverage reports failed!" << std::endl;
    return std::nullopt;
  }

  return aggregated;
}

}  // namespace

bool ResultsAgregationStage::process(const std::string&,
                                     const std::string& libName,
                                     const cider::Cmd& cmd) {
  const auto& original = getInput().actions;

  Results newResults;
  auto results = getResults();
  std::cout << "[INFO] Got " << results.size() << " methods to process\n";

  const auto& testSets = groupIntoTestSets(results, libName);
  for (const auto& it : testSets) {
    const auto& method = it.first;
    const auto& pack = it.second;
    const auto& methodStats = results[method];

    std::cout << "[PROCESS] Method: " << method
              << " | sessions: " << pack.size() << std::endl;

    std::vector<Result> aggregated;
    for (size_t i = 0; i < pack.size(); ++i) {
      std::cout << "  [SESSION] " << i + 1 << "/" << pack.size() << std::endl;

      if (pack[i].size() == 1 && pack[i][0].testCaseName == libName) {
        std::cout << "    [INFO] Pre-aggregated Result detected, skipping "
                     "aggregation\n";
        aggregated.emplace_back(pack[i][0]);
        continue;
      }

      std::cout << "    [INFO] Running aggregation for session..." << std::endl;
      auto aggrRes = aggregateSession(original, libName, cmd, pack[i]);
      if (aggrRes.has_value()) {
        aggregated.emplace_back(aggrRes.value());
      } else {
        std::cout
            << "    [WARN] First pass aggregation failed, retry with recheck\n";
        aggrRes = aggregateSession(original, libName, cmd, pack[i], true);
        if (aggrRes.has_value()) {
          aggregated.emplace_back(aggrRes.value());
        } else {
          std::cout << "    [ERROR] Aggregation failed: " << method
                    << std::endl;
        }
      }
    }

    auto& newEntry = newResults[method];
    newEntry.entries = std::move(aggregated);

    std::cout << "[DONE] Method: " << method << std::endl;
  }

  std::cout << "[INFO] Aggregation stage finished successfully\n";
  replaceResults(newResults);
  return true;
}

}  // namespace pipelines
}  // namespace cider
