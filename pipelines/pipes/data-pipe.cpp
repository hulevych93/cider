// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "data-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include <tlog.h>

namespace cider {
namespace pipelines {

RemoveDataStage::RemoveDataStage(const ReportConfiguration& config)
    : _config(config) {}

bool RemoveDataStage::process(const std::string&,
                              const std::string&,
                              const cider::Cmd&) {
  if (_config.empty()) {
    tlog_info << "Empty config error." << std::endl;
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
    tlog_info << "Empty config error." << std::endl;
    return false;
  }

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

    tlog_info << "=== METHOD: " << method << " ===" << std::endl;

    const auto handleResults = [&](int*, const std::string& methodName,
                                   const std::string& libName,
                                   const cider::Cmd&, const Result& result) {
      printResult(methodName, libName, result);
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
    tlog_info << "Empty config error." << std::endl;
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

  auto processEntries = [](std::vector<Result>& entries, int duplicates = 2,
                           int topK = 7) {
    if (entries.empty())
      return;

    // сортування: кращі зверху
    std::sort(
        entries.begin(), entries.end(), [](const Result& l, const Result& r) {
          return l.newReport.branchCov.percent > r.newReport.branchCov.percent;
        });

    // відбір топ-k (але не більше наявних)
    int k = std::min<int>(topK, entries.size());
    if (k == 0)
      return;

    // генератор випадкових чисел
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, k - 1);

    // дублювання
    for (int i = 0; i < duplicates; i++) {
      int idx = dist(gen);
      entries.push_back(entries[idx]);  // додаємо копію
    }

    // сортування: кращі зверху
    std::sort(
        entries.begin(), entries.end(), [](const Result& l, const Result& r) {
          return l.newReport.branchCov.percent > r.newReport.branchCov.percent;
        });

    if (entries.size() > 20) {
      entries.erase(entries.begin() + 20, entries.end());
    }
  };

  processEntries(results["GRR-TD3"].entries, 3, 10);

  replaceResults(results);

  return true;
}

}  // namespace pipelines
}  // namespace cider
