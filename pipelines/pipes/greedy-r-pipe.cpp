// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "greedy-r-pipe.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#ifdef ENABLE_MATHPLOT
#include "mathplot-log/monitoring/metasearch-basic-block-cov-plot.h"
#endif

#include <assert.h>

namespace cider {
namespace pipelines {

namespace {

template <typename CoverageMetricsType, typename SettingsType>
bool makePipeline(SettingsType settings,
                  const std::string& metadata,
                  const std::string& libName,
                  const cider::Cmd& cmd,
                  const recorder::Actions& input,
                  recorder::Actions& output) {
  try {
    CoverageMetricsType measurer{cmd, libName.c_str()};

    std::stringstream os;
    os << settings;
    auto prefix = os.str();

    settings.objFunc = measurer.getObjValueFunc();

    std::filesystem::path outPath(cmd.resultsDir);
    outPath /= metadata;
    outPath /= prefix;

    measurer.setLogger(outPath.string(), "meta_log.txt");

    output = greedy_r::run_greedy_r(Seed::instance().get(), settings, input);

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return false;
  }

  return true;
}

}  // namespace

GreedyRStage::GreedyRStage(const greedy_r::GreedyRSettings& settings,
                           int numberOfRuns)
    : m_settings(settings), _numberOfRuns(numberOfRuns) {}

bool GreedyRStage::process(const std::string& metadata,
                           const std::string& libName,
                           const cider::Cmd& cmd) {
  bool success = false;
  const auto& input = getInput();

  for (int idx = 0; idx < _numberOfRuns; ++idx) {
    auto start = std::chrono::steady_clock::now();
    recorder::Actions output;

    success = makePipeline<cfg_coverage::CoverageMeasurment>(
        m_settings, metadata, libName, cmd, input.actions, output);

    auto end = std::chrono::steady_clock::now();
    unsigned long elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    Result result;
    result.testCaseName = input.testOrLibName;
    result.timeElapsedMs = elapsed_ms;
    result.oldActions = deepCopy(input.actions);
    result.newActions = deepCopy(output);

    pushResult(libName.c_str(), cmd, getConfigName(), result);
  }

  return success;
}

std::string GreedyRStage::getConfigName() const {
  return m_settings.configName;
}

}  // namespace pipelines
}  // namespace cider
