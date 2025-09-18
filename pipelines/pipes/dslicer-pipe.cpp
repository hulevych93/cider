// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "dslicer-pipe.h"

#include <iostream>

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

namespace cider {
namespace pipelines {

namespace {

template <typename SettingsType>
bool runDynamicSlicing(SettingsType settings,
                       const std::string& libName,
                       const cider::Cmd& cmd,
                       const recorder::Actions& input,
                       recorder::Actions& output) {
  try {
    gcov_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
    cfg_coverage::CoverageMeasurment fastMeasurer{cmd, libName.c_str()};

    if constexpr (std::is_same_v<SettingsType, dslicer::DSlicingSettings>) {
      settings.objFunc = measurer.getObjValueFunc();
      output = dslicer::run_d_slicing(settings, input);
    } else if constexpr (std::is_same_v<SettingsType,
                                        dslicer::FastDSlicingSettings>) {
      settings.objFunc = measurer.getObjValueFunc();
      settings.fineObjFunc = fastMeasurer.getFastObjValueFunc();
      output = dslicer::run_d_slicing_fast_checked(settings, input);
    } else if constexpr (std::is_same_v<
                             SettingsType,
                             dslicer::FastMultiPassDSlicingSettings>) {
      settings.objFunc = measurer.getObjValueFunc();
      settings.fineObjFunc = fastMeasurer.getFastObjValueFunc();

      const auto report = measurer.getReport(input);
      if (report.has_value()) {
        const auto baseline = getOldCov(libName, report->report);
        settings.baseline = baseline;
      }

      output = dslicer::run_d_slicing_fast_multipass(settings, input);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return false;
  }

  return true;
}

}  // namespace

DSlicerStage::DSlicerStage(const dslicer::DSLSettings& settings,
                           int numberOfRuns)
    : _settings(settings), _numberOfRuns(numberOfRuns) {}

bool DSlicerStage::process(const std::string&,
                           const std::string& libName,
                           const cider::Cmd& cmd) {
  bool success = false;
  const auto& input = getInput();

  for (int idx = 0; idx < _numberOfRuns; ++idx) {
    auto start = std::chrono::steady_clock::now();
    recorder::Actions output;

    success = std::visit(
        [&](const auto& settings) -> bool {
          return runDynamicSlicing(settings, libName, cmd,
                                   deepCopy(input.actions), output);
        },
        _settings);

    auto end = std::chrono::steady_clock::now();
    unsigned long elapsed_mcs =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();

    Result result;
    result.testCaseName = input.testOrLibName;
    result.timeElapsedMcs = elapsed_mcs;
    result.oldActions = deepCopy(input.actions);
    result.newActions = deepCopy(output);

    const auto slicerMethod = std::visit(
        [](const auto& settings) { return settings.configName; }, _settings);

    pushResult(libName.c_str(), cmd, slicerMethod, result);
  }

  return success;
}

DQLPostProcessSlicerStage::DQLPostProcessSlicerStage(
    const dslicer::DSLSettings& settings,
    const ReportConfiguration& config)
    : _config(config), _settings(settings) {}

bool DQLPostProcessSlicerStage::process(const std::string&,
                                        const std::string& libName,
                                        const cider::Cmd& cmd) {
  auto results = getResults();

  std::cout << "[INFO] PostProcessDSlicerStage: start\n";

  int i = 0;
  for (const auto& methodName : _config) {
    auto it = results.find(methodName);
    if (it == results.end()) {
      std::cout << "  [WARN] Method not found: " << methodName << "\n";
      continue;
    }

    const auto& methodResults = it->second.entries;

    const auto slicerMethod = std::visit(
        [](const auto& settings) { return settings.configName; }, _settings);

    std::string newMethodName = methodName + "+" + slicerMethod;

    std::cout << "  [PROCESS] " << methodName << " -> " << newMethodName
              << " | entries: " << getDataSize(libName) << "\n";

    int j = 0;
    const auto handleResult = [this, &i, &j](
                                  int*, const std::string& methodName,
                                  const std::string& libName,
                                  const cider::Cmd& cmd, const Result& r) {
      auto start = std::chrono::steady_clock::now();
      recorder::Actions sliced;
      auto newActions = deepCopy(r.newActions);

      std::visit(
          [&](const auto& settings) -> bool {
            return runDynamicSlicing(settings, libName, cmd, newActions,
                                     sliced);
          },
          _settings);

      if (sliced.empty()) {
        return;
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

      pushResult(libName.c_str(), cmd, methodName, result);

      std::cout << "[" << i << "," << _config.size() << "]";
      std::cout << "[" << j << "," << getDataSize(libName) << "]";

      std::cout << "    [OK] " << r.testCaseName
                << " | oldLen=" << r.newActions.size()
                << " -> newLen=" << sliced.size() << " | time=" << elapsed_mcs
                << " mcs\n";
      j++;
    };

    processBest((int*)(nullptr), newMethodName, libName, cmd, methodResults,
                getDataSize(libName), handleResult);

    i++;
  }

  std::cout << "[INFO] PostProcessDSlicerStage: done\n";
  return true;
}

}  // namespace pipelines
}  // namespace cider
