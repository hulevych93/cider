// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "compression-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "pipelines/metrics.h"

#include <iostream>
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
    std::cout << "Empty config error." << std::endl;
    return false;
  }

  int i = 0;
  auto results = getResults();
  for (const auto& methodConfig : _config) {
    auto it = results.find(methodConfig);
    if (it == results.end()) {
      std::cout << "Warning method not simulated: " << methodConfig
                << std::endl;
      continue;
    }

    auto& method = it->first;
    auto& pack = it->second;

    i++;

    std::cout << "  [METHOD] " << method
              << " | entries: " << pack.entries.size() << std::endl;

    int j = 0;
    for (auto& r : pack.entries) {
      j++;
      auto reached = computeCoverageReachedLength(r.oldActions, r.newActions,
                                                  libName, cmd);
      std::cout << "[" << i << "," << results.size() << "][" << j << ","
                << pack.entries.size() << "]" << std::endl;
      if (reached.has_value()) {
        r.coverageReachedLength = *reached;
        std::cout << "[OK] " << r.testCaseName
                  << " coverage reached at length = " << *reached << "\n";
      } else {
        r.coverageReachedLength = r.newActions.size();
        std::cout << "[FAIL] " << r.testCaseName
                  << " never reached old coverage, "
                  << "fallback = full length "
                  << r.coverageReachedLength.value() << "\n";
      }
    }
  }

  replaceResults(results);
  return true;
}

}  // namespace pipelines
}  // namespace cider
