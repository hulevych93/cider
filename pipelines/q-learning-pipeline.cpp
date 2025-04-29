// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning-pipeline.h"

#include <iostream>
#include "recorder/details/generator.h"

#include "q-learning/agent.h"
#include "q-learning/q-learning.h"
#include "q-learning/scenario.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

using namespace cider::qleaning;

namespace cider {
namespace pipelines {

namespace {

Settings getSettings(std::string& prefix) {
  Settings settings;
  settings.discountFactor = 0.85;
  settings.learningRate = 0.05;
  settings.episodes = 100U;

  std::stringstream os;
  os << settings;
  prefix = os.str();
  return settings;
}

int qlearningPipeline(
    const Settings& settings,
    const std::string& resultsDir,
    const std::string& libName,
    const std::string& prefix,
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions) {
  try {
    QValuesAgent agent;

    for (const auto& session : sessions) {
      std::cout << "InstructionsCount: " << session->getInstructionsCount()
                << std::endl;

      const auto& initial = session->getInstructions();

      learningSession(settings, initial, agent);
    }

    std::filesystem::path outPath(resultsDir);
    std::filesystem::create_directories(outPath);
    std::ofstream debug(outPath / (prefix + "qtable_cfg.txt"));

    std::ostringstream oss;

    const auto& actions = sessions[0]->getInstructions();
    for (size_t i = 0; i < actions.size(); ++i) {
      oss << actionToShortString(actions[i]) << "\t";
      print(oss, actions[i]);
      oss << std::endl;
    }

    debug << oss.str();
    debug << "Coverage: " << settings.objFunc(actions);
    debug << std::endl << std::endl;

    auto generator = cider::recorder::makeLuaGenerator(libName);
    debug << "Script:\n "
          << cider::recorder::generateScript(generator, actions, 999999U);

    debug << std::endl << std::endl;
    agent.print(debug);

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}

}  // namespace

int qlearningGcovrPipeline(
    const std::string& libName,
    const cider::Cmd& cmd,
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions) {
  std::string prefix;
  auto settings = getSettings(prefix);

  cider::gcov_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  auto fileLog = std::make_unique<cider::gcov_coverage::FileLogger>(
      cmd.resultsDir, prefix + "_gcov_qlearning_log.txt");
  measurer.setLogger(std::move(fileLog));

  const auto objFunc =
      [&](const std::vector<cider::recorder::Action>& actions) -> double {
    const auto rootReport = measurer(actions);
    if (rootReport.has_value()) {
      return rootReport->report.lineCov.percent;
    }
    return 0.0f;
  };

  settings.objFunc = objFunc;

  return qlearningPipeline(settings, cmd.resultsDir, libName, prefix, sessions);
}

int qlearningCfgPipeline(
    const std::string& libName,
    const cider::Cmd& cmd,
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions) {
  std::string prefix;
  auto settings = getSettings(prefix);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  auto fileLog = std::make_unique<cider::cfg_coverage::FileLogger>(
      cmd.resultsDir, prefix + "_cfg_qlearning_log.txt");
  measurer.setLogger(std::move(fileLog));

  const auto objFunc =
      [&](const std::vector<cider::recorder::Action>& actions) -> double {
    const auto rootReport = measurer(actions);
    if (rootReport.has_value()) {
      return rootReport->getPercentage();
    }
    return 0.0f;
  };

  settings.objFunc = objFunc;

  return qlearningPipeline(settings, cmd.resultsDir, libName, prefix, sessions);
}

}  // namespace pipelines
}  // namespace cider
