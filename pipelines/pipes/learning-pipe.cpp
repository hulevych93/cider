// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "learning-pipe.h"

#include "agent-model/reward_counter.h"

#include "agent-q-learning/q-learning.h"
#include "agent-sarsa-learning/sarsa-learning.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#ifdef ENABLE_MATHPLOT
#include "mathplot-log/monitoring/q-learning-reward-loss-plot.h"
#endif

#include <assert.h>
#include <iostream>

namespace cider {
namespace pipelines {

namespace {

void getPrefix(const agent_model::LearningSettings& settings,
               std::string& prefix) {
  std::stringstream os;
  std::visit([&os](const auto& s) { os << "_" << s; }, settings);
  prefix = os.str();
}

}  // namespace

PreLearningStage::PreLearningStage(
    const agent_model::LearningSettings& settings)
    : m_settings(settings) {}

bool PreLearningStage::process(const std::string& metadata,
                               const std::string& libName,
                               const cider::Cmd& cmd) {
  std::string prefix;
  getPrefix(m_settings, prefix);

  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  outPath /= prefix;
  std::filesystem::create_directories(outPath);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_pre_learning_log.txt");

  const auto& input = getInput();

  for (int i = 0; i < 50; ++i) {
    std::visit(
        [&](const auto& settings) {
          prelearningSession(settings, input.actions,
                             measurer.getObjValueFunc());
        },
        m_settings);
  }
  return true;
}

LearningStage::LearningStage(const agent_model::LearningSettings& settings)
    : m_settings(settings) {}

bool LearningStage::process(const std::string& metadata,
                            const std::string& libName,
                            const cider::Cmd& cmd) {
  std::string prefix;
  getPrefix(m_settings, prefix);

  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  outPath /= prefix;
  std::filesystem::create_directories(outPath);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_qlearning_log.txt");

  const auto& input = getInput();

  std::ofstream debug(outPath / "qtable_debug.txt", std::ios::trunc);

  mathplot::QLearningResultsMathplotLogger logger(outPath, "reward_loss.png");

  std::ostringstream oss;

  for (size_t i = 0; i < input.actions.size(); ++i) {
    oss << agent_model::actionToGenericRepro(input.actions[i]) << "\t";
    print(oss, input.actions[i]);
    oss << std::endl;
  }

  debug << oss.str();
  debug << "Input Coverage: "
        << measurer.getObjValueFunc()(input.actions).coverage;
  debug << std::endl << std::endl;

  agent_model::RewardCounter rwCounter;
  auto dump = [&rwCounter, outPath](const auto& agent) {
    std::ofstream debug(outPath / "qtable.txt", std::ios::trunc);
    agent.print(debug);

    agent_model::print(debug, rwCounter);

    agent.save(outPath / "qtable_agent.img");
  };

  std::visit(
      [&](const auto& settings) {
        learningSession(settings, input.actions, measurer.getObjValueFunc(),
                        rwCounter, logger, dump);
      },
      m_settings);

  logger.plot();

  debug << std::endl << std::endl;

  return true;
}

}  // namespace pipelines
}  // namespace cider
