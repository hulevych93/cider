// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "simulation-pipe.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include <assert.h>
#include <iostream>

namespace cider {
namespace pipelines {

SimulationPipe::SimulationPipe(int numberOfRuns)
    : _numberOfRuns(numberOfRuns) {}

bool SimulationPipe::process(const std::string& metadata,
                             const std::string& libName,
                             const cider::Cmd& cmd) {
  std::string prefix = getPrefix();

  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  outPath /= prefix;
  std::filesystem::create_directories(outPath);

  const auto& input = getInput();

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_generation_log.txt");

  gcov_coverage::CoverageMeasurment measurerGcov{cmd, libName.c_str()};

  double baseline = 0.0f;
  const auto report = measurerGcov.getReport(input.actions);
  if (report.has_value()) {
    baseline = getOldCov(libName, report->report);
  }

  bool success = true;
  for (int idx = 0; idx < _numberOfRuns;) {
    recorder::Actions output;
    const auto start = std::chrono::steady_clock::now();

    try {
      success = simulate(outPath, baseline, input.actions, output,
                         measurerGcov.getObjValueFunc(),
                         measurer.getFastObjValueFunc());
    } catch (const std::exception& e) {
      std::cerr << e.what();
      success = false;
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

    if (pushResult(libName.c_str(), cmd, getConfigName(), result)) {
      idx++;
    }

    std::cout << "[" << idx << "," << _numberOfRuns
              << "]: " << (double)elapsed_mcs / 1000 << " ms elapsed"
              << std::endl;
  }

  return success;
}

}  // namespace pipelines
}  // namespace cider
