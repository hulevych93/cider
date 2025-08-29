// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "dslicer-pipe.h"

#include <iostream>

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

namespace cider {
namespace pipelines {

bool DSlicerStage::process(const std::string&,
                           const std::string& libName,
                           const cider::Cmd& cmd) {
  const auto& input = getInput();

  auto start = std::chrono::steady_clock::now();
  recorder::Actions output;

  try {
    gcov_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
    output = dslicer::run_d_slicing(measurer.getObjValueFunc(), input.actions);
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return false;
  }

  auto end = std::chrono::steady_clock::now();
  unsigned long elapsed_mcs =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();

  Result result;
  result.testCaseName = input.testOrLibName;
  result.timeElapsedMcs = elapsed_mcs;
  result.oldActions = deepCopy(input.actions);
  result.newActions = deepCopy(output);

  pushResult(libName.c_str(), cmd, "DSL", result);

  return true;
}

DQLPostProcessSlicerStage::DQLPostProcessSlicerStage(
    const ReportConfiguration& config)
    : _config(config) {}

bool DQLPostProcessSlicerStage::process(const std::string&,
                                        const std::string& libName,
                                        const cider::Cmd& cmd) {
  auto results = getResults();

  std::cout << "[INFO] PostProcessDSlicerStage: start\n";
  for (const auto& methodName : _config) {
    auto it = results.find(methodName);
    if (it == results.end()) {
      std::cout << "  [WARN] Method not found: " << methodName << "\n";
      continue;
    }

    const auto& methodResults = it->second.entries;
    std::string newMethodName = methodName + "+DSL";

    std::cout << "  [PROCESS] " << methodName << " -> " << newMethodName
              << " | entries: " << methodResults.size() << "\n";

    for (const auto& r : methodResults) {
      auto start = std::chrono::steady_clock::now();
      recorder::Actions sliced;
      auto newActions = deepCopy(r.newActions);

      try {
        gcov_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
        // запускаємо d-slicing на newActions!
        sliced = dslicer::run_d_slicing(measurer.getObjValueFunc(), newActions);
      } catch (const std::exception& e) {
        std::cerr << "[ERROR] D-Slicing failed: " << e.what() << "\n";
        continue;
      }

      auto end = std::chrono::steady_clock::now();
      unsigned long elapsed_mcs =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count();

      Result result;
      result.testCaseName = r.testCaseName;
      result.timeElapsedMcs = r.timeElapsedMcs + elapsed_mcs;
      result.oldActions = deepCopy(r.oldActions);
      result.newActions = deepCopy(sliced);

      pushResult(libName.c_str(), cmd, newMethodName, result);

      std::cout << "    [OK] " << r.testCaseName
                << " | oldLen=" << r.newActions.size()
                << " -> newLen=" << sliced.size() << " | time=" << elapsed_mcs
                << " mcs\n";
    }
  }

  std::cout << "[INFO] PostProcessDSlicerStage: done\n";
  return true;
}

}  // namespace pipelines
}  // namespace cider
