// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning-pipe.h"

#include "recorder/details/generator.h"

#include "q-learning/agent.h"
#include "q-learning/q-learning.h"
#include "q-learning/scenario.h"

#include "coverage/cfg_measurer.h"

#include <iostream>

using namespace cider::qleaning;

namespace cider {
namespace pipelines {

namespace {

LearningSettings getLearningSettings(std::string& prefix) {
  LearningSettings settings;
  settings.discountFactor = 0.85;
  settings.learningRate = 0.1;
  settings.episodes = 500U;
  settings.maxRollback = 20U;

  std::stringstream os;
  os << settings;
  prefix = os.str();
  return settings;
}

GenerationSettings getGeneratorSettings(std::string& prefix) {
  GenerationSettings settings;
  settings.epsilon = 0.1;
  settings.maxRollback = 50U;
  settings.strategy = GenerationStrategyType::Greedy;

  std::stringstream os;
  os << "_" << settings;
  prefix = os.str();
  return settings;
}

bool qlearningPipeline(QValuesAgent& agent,
                       const LearningSettings& learningSettings,
                       const std::string& resultsDir,
                       const std::string& libName,
                       const std::string& prefix,
                       const Actions& input) {
  try {
    std::filesystem::path outPath(resultsDir);
    std::filesystem::create_directories(outPath);
    std::ofstream debug(outPath / (prefix + "_qtable_cfg.txt"));

    learningSession(learningSettings, input, agent);

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

    debug << std::endl << std::endl;
    agent.print(debug);

    agent.save(resultsDir + '/' + (prefix + "agent.img"));

    agent.getLogger().save(outPath /= "graph.png");

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return false;
  }

  return true;
}

bool qlearningGenerationPipeline(QValuesAgent& agent,
                                 const GenerationSettings& generatorSettings,
                                 const std::string& resultsDir,
                                 const std::string& libName,
                                 const std::string& prefix,
                                 const Actions& input,
                                 Actions& output) {
  try {
    std::filesystem::path outPath(resultsDir);
    std::filesystem::create_directories(outPath);
    std::ofstream debug(outPath / (prefix + "geneation_log.txt"));

    gererationSession(generatorSettings, agent, input, output);

    auto generator = cider::recorder::makeLuaGenerator(libName);
    debug << "Script generated:\n "
          << cider::recorder::generateScript(generator, input, 999999U);

    debug << "Coverage: " << generatorSettings.objFunc(output).coverage;
    debug << std::endl << std::endl;

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return false;
  }

  return true;
}

}  // namespace

QPreLearningStage::QPreLearningStage() : m_agent(QValuesAgent::getInstance()) {}

bool QPreLearningStage::process(const std::string& metadata,
                                const std::string& libName,
                                const cider::Cmd& cmd,
                                const Actions& input,
                                Actions& output) {
  output = input;

  if (m_agent.isLoaded()) {
    std::cout << "Skip QPreLearningStage..." << std::endl;
    return true;
  }

  std::string prefix;
  auto settings = getLearningSettings(prefix);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(cmd.resultsDir + '/' + metadata,
                     prefix + "_cfg_pre_qlearning_log.txt");

  settings.objFunc = measurer.getObjValueFunc();

  prelearningSession(settings, input, m_agent);

  return true;
}

QLearningStage::QLearningStage() : m_agent(QValuesAgent::getInstance()) {}

bool QLearningStage::process(const std::string& metadata,
                             const std::string& libName,
                             const cider::Cmd& cmd,
                             const Actions& input,
                             Actions& output) {
  output = input;

  std::string prefix;
  auto settings = getLearningSettings(prefix);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(cmd.resultsDir + '/' + metadata,
                     prefix + "_cfg_qlearning_log.txt");

  settings.objFunc = measurer.getObjValueFunc();

  return qlearningPipeline(m_agent, settings, cmd.resultsDir + '/' + metadata,
                           libName, prefix, input);
}

QGenerationStage::QGenerationStage() : m_agent(QValuesAgent::getInstance()) {}

bool QGenerationStage::process(const std::string& metadata,
                               const std::string& libName,
                               const cider::Cmd& cmd,
                               const Actions& input,
                               Actions& output) {
  std::string prefix;
  auto settings = getGeneratorSettings(prefix);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(cmd.resultsDir + '/' + metadata,
                     prefix + "_cfg_qlearning_generation_log.txt");

  settings.objFunc = measurer.getObjValueFunc();
  return qlearningGenerationPipeline(m_agent, settings,
                                     cmd.resultsDir + '/' + metadata, libName,
                                     prefix, input, output);
}

}  // namespace pipelines
}  // namespace cider
