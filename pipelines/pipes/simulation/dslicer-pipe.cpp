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
                       const ObjectiveFunction& objFunc,
                       const FineObjectiveFunction& fineObjFunc,
                       const recorder::Actions& input,
                       recorder::Actions& output,
                       double baseline) {
  try {
    if constexpr (std::is_same_v<SettingsType, dslicer::DSlicingSettings>) {
      settings.objFunc = objFunc;
      settings.baseline = baseline;

      output = dslicer::run_d_slicing(settings, input);
    } else if constexpr (std::is_same_v<SettingsType,
                                        dslicer::FastDSlicingSettings>) {
      settings.objFunc = objFunc;
      settings.fineObjFunc = fineObjFunc;
      settings.baseline = baseline;

      output = dslicer::run_d_slicing_fast_checked(settings, input);
    } else if constexpr (std::is_same_v<
                             SettingsType,
                             dslicer::FastMultiPassDSlicingSettings>) {
      settings.objFunc = objFunc;
      settings.fineObjFunc = fineObjFunc;
      settings.baseline = baseline;

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
    : SimulationPipe(numberOfRuns), _settings(settings) {}

bool DSlicerStage::simulate(const std::string& /*outPath*/,
                            const double baseline,
                            const recorder::Actions& input,
                            recorder::Actions& output,
                            const ObjectiveFunction& objFunc,
                            const FineObjectiveFunction& fineObjFunc) {
  return std::visit(
      [&](const auto& settings) -> bool {
        return runDynamicSlicing(settings, objFunc, fineObjFunc,
                                 deepCopy(input), output, baseline);
      },
      _settings);
}

std::string DSlicerStage::getPrefix() const {
  std::stringstream os;
  std::visit([&os](const auto& s) { os << "_" << s; }, _settings);
  return os.str();
}

std::string DSlicerStage::getConfigName() const {
  return std::visit([](const auto& settings) { return settings.configName; },
                    _settings);
}

DQLPostProcessSlicerStage::DQLPostProcessSlicerStage(
    const dslicer::DSLSettings& settings,
    const ReportConfiguration& config)
    : _config(config), _settings(settings) {}

bool DQLPostProcessSlicerStage::process(const std::string&,
                                        const std::string& libName,
                                        const cider::Cmd& cmd) {
  auto results = getResults();
  const auto& input = getInput();

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

    gcov_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
    cfg_coverage::CoverageMeasurment fastMeasurer{cmd, libName.c_str()};

    double baseline = 0.0f;
    const auto report = measurer.getReport(input.actions);
    if (report.has_value()) {
      baseline = getOldCov(libName, report->report);
    }

    int j = 0;
    const auto handleResult =
        [this, &i, &j, &fastMeasurer, &measurer, baseline](
            int*, const std::string& methodName, const std::string& libName,
            const cider::Cmd& cmd, const Result& r) {
          auto start = std::chrono::steady_clock::now();
          recorder::Actions sliced;
          auto newActions = deepCopy(r.newActions);

          std::visit(
              [&](const auto& settings) -> bool {
                return runDynamicSlicing(settings, measurer.getObjValueFunc(),
                                         fastMeasurer.getFastObjValueFunc(),
                                         newActions, sliced, baseline);
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
                    << " -> newLen=" << sliced.size()
                    << " | time=" << elapsed_mcs << " mcs\n";
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
