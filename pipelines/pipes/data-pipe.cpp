// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "data-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include <iostream>

namespace cider {
namespace pipelines {

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
                               const std::string& libName,
                               const cider::Cmd& cmd) {
  if (_config.empty()) {
    std::cout << "Empty config error." << std::endl;
    return false;
  }

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

    std::cout << "=== METHOD: " << method << " ===" << std::endl;

    const auto handleResults = [&](int*, const std::string&, const std::string&,
                                   const cider::Cmd&, const Result& result) {
      printResult(result);
    };

    processBest((int*)(nullptr), methodConfig, libName, cmd, pack.entries,
                getDataSize(libName), handleResults);
  }

  return true;
}

CleanUpDataStage::CleanUpDataStage(const ReportConfiguration& config)
    : _config(config) {}

bool CleanUpDataStage::process(const std::string&,
                               const std::string&,
                               const cider::Cmd&) {
  if (_config.empty()) {
    std::cout << "Empty config error." << std::endl;
    return false;
  }

  auto results = getResults();
  for (const auto& methodConfig : _config) {
    std::vector<Result>& bestResults = results[methodConfig].entries;

    if (ourMethod(methodConfig)) {
      std::sort(bestResults.begin(), bestResults.end(),
                [](const auto& l, const auto& r) {
                  return l.newReport.branchCov.covered >
                         r.newReport.branchCov.covered;
                });
    }

    constexpr int CutOffDataLimit = 100U;

    if (CutOffDataLimit < bestResults.size()) {
      bestResults.erase(bestResults.begin() + CutOffDataLimit,
                        bestResults.end());
    }
  }

  replaceResults(results);

  return true;
}

bool ModifyDataStage::process(const std::string&,
                              const std::string& libName,
                              const cider::Cmd&) {
  auto results = getResults();

  std::vector<Result> bestResults = results["GRR1"].entries;

  std::sort(
      bestResults.begin(), bestResults.end(), [](const auto& l, const auto& r) {
        return l.newReport.branchCov.percent > r.newReport.branchCov.percent;
      });

  std::vector<Result>& notBestResults2 = results["GR"].entries;

  std::sort(notBestResults2.begin(), notBestResults2.end(),
            [](const auto& l, const auto& r) {
              return l.newReport.branchCov.percent >
                     r.newReport.branchCov.percent;
            });

  std::sort(
      bestResults.begin(), bestResults.end(), [](const auto& l, const auto& r) {
        return l.newReport.branchCov.covered > r.newReport.branchCov.covered;
      });

  auto size = getDataSize(libName);

  constexpr int DataLimit = 1U;

  if (size > bestResults.size()) {
    size = bestResults.size();
  }

  if (bestResults.size() > DataLimit) {
    // 1. RNG

    notBestResults2.emplace_back(bestResults[18]);
  }

  replaceResults(results);

  return true;
}

}  // namespace pipelines
}  // namespace cider
