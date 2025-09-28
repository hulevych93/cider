// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "dslicer-pipe.h"

#include <tlog.h>

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
                                        dslicer::BatchDSlicingSettings>) {
      settings.objFunc = objFunc;
      settings.fineObjFunc = fineObjFunc;
      settings.baseline = baseline;

      output = dslicer::run_d_slicing_batch(settings, input);
    } else if constexpr (std::is_same_v<
                             SettingsType,
                             dslicer::BatchMultiPassDSlicingSettings>) {
      settings.objFunc = objFunc;
      settings.fineObjFunc = fineObjFunc;
      settings.baseline = baseline;

      output = dslicer::run_d_slicing_batch_multipass(settings, input);
    } else if constexpr (std::is_same_v<SettingsType,
                                        dslicer::BatchTracksDSlicingSettings>) {
      settings.objFunc = objFunc;
      settings.fineObjFunc = fineObjFunc;
      settings.baseline = baseline;

      output = dslicer::run_d_slicing_fast_tracks(settings, input);
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
                            const ObjectiveFunction& /*objFuncСfg*/,
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

  tlog_info << "[INFO] PostProcessDSlicerStage: start\n";

  int i = 0;
  for (const auto& methodName : _config) {
    auto it = results.find(methodName);
    if (it == results.end()) {
      tlog_info << "  [WARN] Method not found: " << methodName << "\n";
      continue;
    }

    const auto& methodResults = it->second.entries;

    const auto slicerMethod = std::visit(
        [](const auto& settings) { return settings.configName; }, _settings);

    std::string newMethodName = methodName + "+" + slicerMethod;

    tlog_info << "  [PROCESS] " << methodName << " -> " << newMethodName
              << " | entries: " << getDataSize(libName) << "\n";

    gcov_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
    cfg_coverage::CoverageMeasurment fastMeasurer{cmd, libName.c_str()};

    int j = 0;
    const auto handleResult = [this, &i, &j, &fastMeasurer, &measurer](
                                  int*, const std::string& methodName,
                                  const std::string& libName,
                                  const cider::Cmd& cmd, const Result& r) {
      auto start = std::chrono::steady_clock::now();
      recorder::Actions sliced;
      auto newActions = deepCopy(r.newActions);

      std::visit(
          [&](const auto& settings) -> bool {
            return runDynamicSlicing(settings, measurer.getObjValueFunc(),
                                     fastMeasurer.getFastObjValueFunc(),
                                     newActions, sliced, 0.0f);
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
      result.timeElapsedMcs = r.timeElapsedMcs;
      result.oldActions = deepCopy(r.oldActions);
      result.newActions = deepCopy(sliced);

      pushResult(libName.c_str(), cmd, methodName, result);

      tlog_info << "[" << i << "," << _config.size() << "]";
      tlog_info << "[" << j << "," << getDataSize(libName) << "]";

      tlog_info << "    [OK] " << r.testCaseName
                << " | oldLen=" << r.newActions.size()
                << " -> newLen=" << sliced.size() << " | time=" << elapsed_mcs
                << " mcs\n";
      j++;
    };

    processBest((int*)(nullptr), newMethodName, libName, cmd, methodResults,
                getDataSize(libName), handleResult);

    i++;
  }

  tlog_info << "[INFO] PostProcessDSlicerStage: done\n";
  return true;
}

}  // namespace pipelines
}  // namespace cider
