// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "mcts-pipe.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "mathplot-log/monitoring/metasearch-basic-block-cov-plot.h"

#include <assert.h>

namespace cider {
namespace pipelines {

namespace {

template <typename CoverageMetricsType, typename SettingsType>
bool makeMctsPipeline(SettingsType settings,
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

    output = mcts::run_mcts(Seed::instance().get(), settings, input);

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return false;
  }

  return true;
}

}  // namespace

MctsSearchStage::MctsSearchStage(const mcts::MonteCarloSettings& settings,
                                 int numberOfRuns)
    : m_settings(settings), _numberOfRuns(numberOfRuns) {}

bool MctsSearchStage::process(const std::string& metadata,
                              const std::string& libName,
                              const cider::Cmd& cmd) {
  bool success = false;
  const auto& input = getInput();

  for (int idx = 0; idx < _numberOfRuns; ++idx) {
    auto start = std::chrono::steady_clock::now();
    recorder::Actions output;

    success = makeMctsPipeline<cfg_coverage::CoverageMeasurment>(
        m_settings, metadata, libName, cmd, input.actions, output);

    auto end = std::chrono::steady_clock::now();
    unsigned long elapsed_mcs =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();

    Result result;
    result.testCaseName = input.testOrLibName;
    result.timeElapsedMcs = elapsed_mcs;
    result.oldActions = deepCopy(input.actions);
    result.newActions = deepCopy(output);

    pushResult(libName.c_str(), cmd, getConfigName(), result);
  }

  return success;
}

std::string MctsSearchStage::getConfigName() const {
  return m_settings.configName;
}

}  // namespace pipelines
}  // namespace cider
