// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning-pipe.h"

#include "recorder/details/generator.h"

#include "q-learning/agent.h"
#include "q-learning/q-learning.h"
#include "q-learning/scenario.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#ifdef ENABLE_MATHPLOT
#include "mathplot-log/monitoring/q-learning-reward-loss-plot.h"
#endif

#include <assert.h>
#include <iostream>

using namespace cider::qleaning;

namespace cider {
namespace pipelines {

namespace {

void getPrefix(const LearningSettings& settings, std::string& prefix) {
  std::stringstream os;
  os << "_" << settings;
  prefix = os.str();
}

bool qlearningPipeline(QAgent& agent,
                       const LearningSettings& learningSettings,
                       const std::string& outDir,
                       const std::string& libName,
                       const recorder::Actions& input) {
  std::filesystem::path outPath(outDir);
  std::ofstream debug(outPath / "qtable_debug.txt", std::ios::trunc);

  mathplot::QLearningResultsMathplotLogger logger(outPath, "reward_loss.png");

  std::ostringstream oss;

  for (size_t i = 0; i < input.size(); ++i) {
    oss << actionToGenericRepro(input[i]) << "\t";
    print(oss, input[i]);
    oss << std::endl;
  }

  debug << oss.str();
  debug << "Coverage: " << learningSettings.objFunc(input).coverage;
  debug << std::endl << std::endl;

  auto generator = cider::recorder::makeLuaGenerator(libName);
  debug << "Script:\n "
        << cider::recorder::generateScript(generator, input, 999999U);

  RewardCounter rwCounter;
  auto dump = [&agent, &rwCounter, outPath]() {
    std::ofstream debug(outPath / "qtable.txt", std::ios::trunc);
    agent.print(debug);

    print(debug, rwCounter);

    agent.save(outPath / "qtable_agent.img");
  };

  learningSession(rwCounter, learningSettings, input, agent, logger, dump);

  logger.plot();

  debug << std::endl << std::endl;

  dump();

  return true;
}

}  // namespace

QPreLearningStage::QPreLearningStage(const qleaning::LearningSettings& settings)
    : m_agent(qleaning::getAgent()), m_settings(settings) {}

bool QPreLearningStage::process(const std::string& metadata,
                                const std::string& libName,
                                const cider::Cmd& cmd) {
  if (m_agent.isLoaded()) {
    std::cout << "Skip QPreLearningStage..." << std::endl;
    return true;
  }

  std::string prefix;
  getPrefix(m_settings, prefix);

  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  outPath /= prefix;
  std::filesystem::create_directories(outPath);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_pre_qlearning_log.txt");

  m_settings.objFunc = measurer.getObjValueFunc();

  const auto& input = getInput();

  for (int i = 0; i < 50; ++i) {
    prelearningSession(m_settings, input.actions, m_agent);
  }
  return true;
}

QLearningStage::QLearningStage(const qleaning::LearningSettings& settings)
    : m_agent(qleaning::getAgent()), m_settings(settings) {}

bool QLearningStage::process(const std::string& metadata,
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

  m_settings.objFunc = measurer.getObjValueFunc();

  const auto& input = getInput();
  return qlearningPipeline(m_agent, m_settings, outPath.string(), libName,
                           input.actions);
}

}  // namespace pipelines
}  // namespace cider
