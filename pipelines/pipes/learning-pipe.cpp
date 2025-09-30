// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "learning-pipe.h"

#include "agent-model/reward_counter.h"

#include "agent-q-learning/q-learning.h"
#include "agent-sarsa-learning/sarsa-learning.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "mathplot-log/monitoring/q-learning-cov-ep-plot.h"
#include "mathplot-log/monitoring/q-learning-reward-loss-plot.h"

#include <assert.h>
#include <tlog.h>

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

  mathplot::QLearningRewardLogger rwLogger(outPath, "prel_reward.png");
  mathplot::QLearningLossLogger lossLogger(outPath, "prel_loss.png");

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_pre_learning_log.txt");

  const auto& input = getInput();

  std::visit(
      [&](const auto& settings) {
        prelearningSession(settings, input.actions, measurer.getObjValueFunc(),
                           rwLogger, lossLogger);
      },
      m_settings);

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

  mathplot::QLearningRewardLogger rwLogger(outPath, "reward.png");
  mathplot::QLearningLossLogger lossLogger(outPath, "loss.png");
  mathplot::CovQLearningResultsMathplotLogger covLogger(outPath, "cov.png");

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

  static int EpisodeCounter = 0;
  cider::agent_model::Episode t;
  t.episode = EpisodeCounter++;
  t.start = std::chrono::system_clock::now();

  agent_model::RewardCounter rwCounter;
  auto dump = [&t, &cmd, &rwCounter](const auto& agent) {
    t.end = std::chrono::system_clock::now();

    std::filesystem::path resultsPath(cmd.resultsDir);

    std::ofstream debug(resultsPath / "qtable.txt", std::ios::trunc);
    agent.print(debug);

    std::ofstream out(resultsPath / "qtable.csv",
                      std::ios::binary | std::ios::app);
    out.imbue(std::locale::classic());
    agent.printMetrics(out, t);

    agent_model::print(debug, rwCounter);

    agent.save();
  };

  std::visit(
      [&](const auto& settings) {
        learningSession(settings, input.actions, measurer.getObjValueFunc(),
                        rwCounter, rwLogger, lossLogger, covLogger, dump);
      },
      m_settings);

  debug << std::endl << std::endl;

  return true;
}

}  // namespace pipelines
}  // namespace cider
