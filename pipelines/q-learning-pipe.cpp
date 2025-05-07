// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning-pipe.h"

#include "recorder/details/generator.h"

#include "q-learning/agent.h"
#include "q-learning/q-learning.h"
#include "q-learning/scenario.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include <iostream>

using namespace cider::qleaning;

namespace cider {
namespace pipelines {

namespace {

LearningSettings getLearningSettings(std::string& prefix) {
  LearningSettings settings;
  settings.discountFactor = 0.85;
  settings.learningRate = 0.15;
  settings.episodes = 20U;
  settings.maxRollback = 5U;

  std::stringstream os;
  os << settings;
  prefix = os.str();
  return settings;
}

GenerationSettings getGeneratorSettings(std::string& prefix) {
  GenerationSettings settings;
  settings.epsilon = 0.1;
  settings.maxRollback = 5U;
  settings.strategy = GenerationStrategyType::EGreedy;

  std::stringstream os;
  os << "_" << settings;
  prefix = os.str();
  return settings;
}

bool qlearningPipeline(QValuesAgent& agent,
                       const LearningSettings& learningSettings,
                       const GenerationSettings& generatorSettings,
                       const std::string& resultsDir,
                       const std::string& libName,
                       const std::string& prefix,
                       const Actions& input,
                       Actions& output) {
  try {
    std::filesystem::path outPath(resultsDir);
    std::filesystem::create_directories(outPath);
    std::ofstream debug(outPath / (prefix + "qtable_cfg.txt"));

    std::cout << "InstructionsCount: " << input.size() << std::endl;

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

    gererationSession(generatorSettings, agent, input, output);

    debug << "Script generated:\n "
          << cider::recorder::generateScript(generator, input, 999999U);

    debug << "Coverage: " << learningSettings.objFunc(output).coverage;
    debug << std::endl << std::endl;

    agent.save(outPath / (prefix + "agent.img"));
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return false;
  }

  return true;
}

bool qlearningGcovrPipeline(QValuesAgent& agent,
                            const std::string& metadata,
                            const std::string& libName,
                            const cider::Cmd& cmd,
                            const Actions& input,
                            Actions& output) {
  std::string prefix;
  auto settings = getLearningSettings(prefix);

  std::string generatorPrefix;
  auto genSettings = getGeneratorSettings(generatorPrefix);
  prefix += generatorPrefix;

  cider::gcov_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  auto fileLog = std::make_unique<cider::gcov_coverage::FileLogger>(
      cmd.resultsDir + '/' + metadata, prefix + "_gcov_qlearning_log.txt");
  measurer.setLogger(std::move(fileLog));

  const auto objFunc = [&](const std::vector<cider::recorder::Action>& actions)
      -> qleaning::ObjectiveValue {
    const auto rootReport = measurer(actions);
    qleaning::ObjectiveValue value;
    if (rootReport.has_value()) {
      value.coverage = rootReport->report.lineCov.percent;
    }
    return value;
  };

  settings.objFunc = objFunc;
  genSettings.objFunc = objFunc;

  return qlearningPipeline(agent, settings, genSettings,
                           cmd.resultsDir + '/' + metadata, libName, prefix,
                           input, output);
}

bool qlearningCfgPipeline(QValuesAgent& agent,
                          const std::string& metadata,
                          const std::string& libName,
                          const cider::Cmd& cmd,
                          const Actions& input,
                          Actions& output) {
  std::string prefix;
  auto settings = getLearningSettings(prefix);

  std::string generatorPrefix;
  auto genSettings = getGeneratorSettings(generatorPrefix);
  prefix += generatorPrefix;

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  auto fileLog = std::make_unique<cider::cfg_coverage::FileLogger>(
      cmd.resultsDir + '/' + metadata, prefix + "_cfg_qlearning_log.txt");
  measurer.setLogger(std::move(fileLog));

  const auto objFunc = [&](const std::vector<cider::recorder::Action>& actions)
      -> qleaning::ObjectiveValue {
    const auto rootReport = measurer(actions);

    qleaning::ObjectiveValue value;
    if (rootReport.has_value()) {
      value.coverage = rootReport->getPercentage();
      value.coveredTracks = rootReport->coveredTracks;
    }
    return value;
  };

  settings.objFunc = objFunc;
  genSettings.objFunc = objFunc;

  return qlearningPipeline(agent, settings, genSettings,
                           cmd.resultsDir + '/' + metadata, libName, prefix,
                           input, output);
}

}  // namespace

bool QLearningStage::process(const std::string& metadata,
                             const std::string& libName,
                             const cider::Cmd& cmd,
                             const Actions& input,
                             Actions& output) {
  return qlearningCfgPipeline(m_agent, metadata, libName, cmd, input, output);
}

}  // namespace pipelines
}  // namespace cider
