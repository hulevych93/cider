// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning-pipeline.h"

#include <iostream>
#include "recorder/details/generator.h"

#include "q-learning/agent.h"
#include "q-learning/q-learning.h"
#include "q-learning/scenario.h"

using namespace cider::qleaning;

namespace cider {
namespace pipelines {

int qlearningPipeline(
    const std::string& libName,
    const cider::coverage::Cmd& cmd,
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions) {
  try {
    QValuesAgent agent;

    int index = 0;
    std::string log = std::to_string(index) + "_qlearning_log.txt";
    cider::coverage::CoverageMeasurment measurer{cmd, log.c_str(),
                                                 libName.c_str()};

    for (const auto& session : sessions) {
      std::cout << "InstructionsCount: " << session->getInstructionsCount()
                << std::endl;

      const auto& initial = session->getInstructions();

      learningSession(measurer, initial, agent, 200U);
    }

    std::ofstream debug("qtree.txt");

    std::ostringstream oss;
    const auto& actions = sessions[0]->getInstructions();
    for (size_t i = 0; i < actions.size(); ++i) {
        oss << actionToShortString(actions[i]) << "-" << actionToFullString(actions[i]) << std::endl;
    }
    debug << oss.str();
    debug << "Coverage: " << std::endl;
    cider::coverage::printTableEntry(debug, 0, measurer(actions).value().report);

    debug << std::endl << std::endl;
    agent.print(debug);

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}

}  // namespace pipelines
}  // namespace cider
