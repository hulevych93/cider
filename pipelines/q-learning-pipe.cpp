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
#include "mathplot-log/mathplot-log.h"
#endif

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

bool qlearningPipeline(QAgent& agent,
                       const LearningSettings& learningSettings,
                       const std::string& outDir,
                       const std::string& libName,
                       const Actions& input) {
  try {
    std::filesystem::path outPath(outDir);
    std::ofstream debug(outPath / "qtable_debug.txt");

#ifdef ENABLE_MATHPLOT
    qleaning::MathplotLogger logger(outPath, "reward_loss.png");
#else
    qleaning::FileLogger logger(outPath, "reward_loss.txt");
#endif

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

    auto dump = [&]() {
      agent.print(debug);
      agent.save(outPath / "qtable_agent.img");
    };

    learningSession(learningSettings, input, agent, logger, dump);

    debug << std::endl << std::endl;

    dump();

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return false;
  }

  return true;
}

bool qlearningGenerationPipeline(QAgent& agent,
                                 const GenerationSettings& generatorSettings,
                                 const std::string& outDir,
                                 const std::string& libName,
                                 const Actions& input,
                                 Actions& output) {
  try {
    std::filesystem::path outPath(outDir);
    std::ofstream debug(outPath / "geneation_log.txt");

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

QPreLearningStage::QPreLearningStage() : m_agent(qleaning::getAgent()) {}

bool QPreLearningStage::process(const std::string& metadata,
                                const std::string& libName,
                                const cider::Cmd& cmd,
                                const Actions& input) {
  if (m_agent.isLoaded()) {
    std::cout << "Skip QPreLearningStage..." << std::endl;
    return true;
  }

  std::string prefix;
  auto settings = getLearningSettings(prefix);

  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  outPath /= prefix;
  std::filesystem::create_directories(outPath);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_pre_qlearning_log.txt");

  settings.objFunc = measurer.getObjValueFunc();

  prelearningSession(settings, input, m_agent);

  return true;
}

QLearningStage::QLearningStage() : m_agent(qleaning::getAgent()) {}

bool QLearningStage::process(const std::string& metadata,
                             const std::string& libName,
                             const cider::Cmd& cmd,
                             const Actions& input) {
  std::string prefix;
  auto settings = getLearningSettings(prefix);

  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  outPath /= prefix;
  std::filesystem::create_directories(outPath);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_qlearning_log.txt");

  settings.objFunc = measurer.getObjValueFunc();

  return qlearningPipeline(m_agent, settings, outPath.string(), libName, input);
}

QGenerationStage::QGenerationStage() : m_agent(qleaning::getAgent()) {}

bool QGenerationStage::process(const std::string& metadata,
                               const std::string& libName,
                               const cider::Cmd& cmd,
                               const Actions& input) {
  std::string prefix;
  auto settings = getGeneratorSettings(prefix);

  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  outPath /= prefix;
  std::filesystem::create_directories(outPath);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_generation_log.txt");

  settings.objFunc = measurer.getObjValueFunc();

  Actions output;
  auto res = qlearningGenerationPipeline(m_agent, settings, outPath.string(),
                                         libName, input, output);
  pushResult(output);

  return res;
}

QRandGenerationStage::QRandGenerationStage() : m_agent(qleaning::getAgent()) {}

bool QRandGenerationStage::process(const std::string& metadata,
                                   const std::string& libName,
                                   const cider::Cmd& cmd,
                                   const Actions& input) {
  std::string prefix;
  GenerationSettings settings;
  settings.strategy = GenerationStrategyType::Random;

  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  outPath /= prefix;
  std::filesystem::create_directories(outPath);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_generation_log.txt");

  settings.objFunc = measurer.getObjValueFunc();

  Actions output;
  auto res = qlearningGenerationPipeline(m_agent, settings, outPath.string(),
                                         libName, input, output);
  pushResult(output);

  return res;
}

bool GenerationReportStage::process(const std::string& metadata,
                                    const std::string& libName,
                                    const cider::Cmd& cmd,
                                    const Actions& input) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

#ifdef ENABLE_MATHPLOT
  auto logger = std::make_shared<cider::gcov_coverage::TripleComparativeLogger>(
      outPath.string(), "triple_comparage.png");
#endif

  auto generator = cider::recorder::makeLuaGenerator(libName);

  {
    cider::gcov_coverage::StepperCoverageMeasurment stepper{cmd,
                                                            libName.c_str()};

    stepper.setLogger(logger);

    stepper.measure(input);
  }

  {
    const auto script =
        cider::recorder::generateScript(generator, input, 99999U);

    std::ofstream output_file(outPath / (libName + ".lua"));
    output_file << script;
  }

  logger->md(1);
  const auto& output = getResults()[0];

  {
    cider::gcov_coverage::StepperCoverageMeasurment stepper{cmd,
                                                            libName.c_str()};

    stepper.setLogger(logger);

    stepper.measure(output);
  }

  {
    const auto script =
        cider::recorder::generateScript(generator, output, 99999U);

    std::ofstream output_file(outPath / (libName + "_opt.lua"));
    output_file << script;
  }

  logger->md(2);
  const auto& randOutput = getResults()[1];

  {
    cider::gcov_coverage::StepperCoverageMeasurment stepper{cmd,
                                                            libName.c_str()};

    stepper.setLogger(logger);

    stepper.measure(randOutput);
  }

  {
    const auto script =
        cider::recorder::generateScript(generator, randOutput, 99999U);

    std::ofstream output_file(outPath / (libName + "_rand.lua"));
    output_file << script;
  }

  return true;
}

}  // namespace pipelines
}  // namespace cider
