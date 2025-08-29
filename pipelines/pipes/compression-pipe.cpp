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

bool ResultsCompressionStage::process(const std::string&,
                                      const std::string& libName,
                                      const cider::Cmd& cmd) {
  auto results = getResults();
  std::cout << "[VERIFY] Got " << results.size() << " methods\n";

  for (auto& it : results) {
    auto& method = it.first;
    auto& pack = it.second;

    std::cout << "  [METHOD] " << method
              << " | entries: " << pack.entries.size() << std::endl;

    for (auto& r : pack.entries) {
      auto reached = computeCoverageReachedLength(r.oldActions, r.newActions,
                                                  libName, cmd);
      if (reached.has_value()) {
        r.coverageReachedLength = *reached;
        std::cout << "    [OK] " << r.testCaseName
                  << " coverage reached at length = " << *reached << "\n";
      } else {
        r.coverageReachedLength = r.newActions.size();
        std::cout << "    [FAIL] " << r.testCaseName
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
