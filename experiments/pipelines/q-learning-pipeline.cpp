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

const double LEARNING_RATE = 0.05;
const double DISCOUNT_FACTOR = 0.85;

template <typename ObjFunc>
int qlearningPipeline(
    const std::string& resultsDir,
    const std::string& libName,
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions,
    const ObjFunc& func) {
  try {
    QValuesAgent agent;

    for (const auto& session : sessions) {
      std::cout << "InstructionsCount: " << session->getInstructionsCount()
                << std::endl;

      const auto& initial = session->getInstructions();

      learningSession(func, initial, agent, 10U, LEARNING_RATE,
                      DISCOUNT_FACTOR);
    }

    std::filesystem::path outPath(resultsDir);
    std::filesystem::create_directories(outPath);
    std::ofstream debug(outPath / "qtable_cfg.txt");

    std::ostringstream oss;
    const auto& actions = sessions[0]->getInstructions();
    for (size_t i = 0; i < actions.size(); ++i) {
      oss << actionToShortString(actions[i]) << "\t";
      print(oss, actions[i]);
      oss << std::endl;
    }
    debug << oss.str();
    debug << "Coverage: " << func(actions);
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
  std::string log = "gcov_qlearning_log.txt";
  cider::gcov_coverage::CoverageMeasurment measurer{cmd, log.c_str(),
                                                    libName.c_str()};

  const auto objFunc =
      [&](const std::vector<cider::recorder::Action>& actions) -> double {
    const auto rootReport = measurer(actions);
    if (rootReport.has_value()) {
      return rootReport->report.lineCov.percent;
    }
    return 0.0f;
  };

  return qlearningPipeline(cmd.resultsDir, libName, sessions, objFunc);
}

int qlearningCfgPipeline(
    const std::string& libName,
    const cider::Cmd& cmd,
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions) {
  std::string log = "cfg_qlearning_log.txt";
  cider::cfg_coverage::CoverageMeasurment measurer{cmd, log.c_str(),
                                                   libName.c_str()};

  const auto objFunc =
      [&](const std::vector<cider::recorder::Action>& actions) -> double {
    const auto rootReport = measurer(actions);
    if (rootReport.has_value()) {
      return rootReport->getPercentage();
    }
    return 0.0f;
  };

  return qlearningPipeline(cmd.resultsDir, libName, sessions, objFunc);
}

}  // namespace pipelines
}  // namespace cider
