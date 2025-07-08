// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "synthesis-pipe.h"

#include "synthesis/agent-synthesis.h"
#include "synthesis/rand-synthesis.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include <assert.h>
#include <iostream>

namespace cider {
namespace pipelines {

namespace {

void getPrefix(const synthesis::SynthesisSettings& settings,
               std::string& prefix) {
  std::stringstream os;
  std::visit([&os](const auto& s) { os << "_" << s; }, settings);
  prefix = os.str();
}

}  // namespace

SynthesisStage::SynthesisStage(const synthesis::SynthesisSettings& settings,
                               int numberOfRuns)
    : m_settings(settings), _numberOfRuns(numberOfRuns) {}

bool SynthesisStage::process(const std::string& metadata,
                             const std::string& libName,
                             const cider::Cmd& cmd) {
  std::string prefix;
  getPrefix(m_settings, prefix);

  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  outPath /= prefix;
  std::filesystem::create_directories(outPath);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_generation_log.txt");

  const auto& input = getInput();

  bool success = true;
  for (int idx = 0; idx < _numberOfRuns; ++idx) {
    recorder::Actions output;
    const auto start = std::chrono::steady_clock::now();

    try {
      success = std::visit(
          [&](const auto& s) {
            return synthesis::synthesize(Seed::instance().get(), s,
                                         measurer.getObjValueFunc(),
                                         input.actions, output);
          },
          m_settings);
    } catch (const std::exception& e) {
      std::cerr << e.what();
      success = false;
    }

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

std::string SynthesisStage::getConfigName() const {
  return std::visit([&](const auto& settings) { return settings.configName; },
                    m_settings);
}

}  // namespace pipelines
}  // namespace cider
