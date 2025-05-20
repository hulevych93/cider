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

void getPrefix(const LearningSettings& settings, std::string& prefix) {
  std::stringstream os;
  os << "_" << settings;
  prefix = os.str();
}

void getPrefix(const GenerationSettings& settings, std::string& prefix) {
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

  RewardCounter rwCounter;
  auto dump = [&agent, &rwCounter, outPath]() {
    std::ofstream debug(outPath / "qtable.txt", std::ios::trunc);
    agent.print(debug);

    print(debug, rwCounter);

    agent.save(outPath / "qtable_agent.img");
  };

  learningSession(rwCounter, learningSettings, input, agent, logger, dump);

  debug << std::endl << std::endl;

  dump();

  return true;
}

bool qlearningGenerationPipeline(QAgent& agent,
                                 const GenerationSettings& generatorSettings,
                                 const std::string& outDir,
                                 const std::string& libName,
                                 const recorder::Actions& input,
                                 recorder::Actions& output) {
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

QGenerationStage::QGenerationStage(const qleaning::GenerationSettings& settings,
                                   int numberOfRuns)
    : m_agent(qleaning::getAgent()),
      m_settings(settings),
      _numberOfRuns(numberOfRuns) {}

bool QGenerationStage::process(const std::string& metadata,
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

  m_settings.objFunc = measurer.getObjValueFunc();

  const auto& input = getInput();

  bool result = true;
  const auto dataName = std::string{"QL"} + m_settings.configName;
  for (int idx = 0; idx < _numberOfRuns; ++idx) {
    recorder::Actions output;
    result &= qlearningGenerationPipeline(m_agent, m_settings, outPath.string(),
                                          libName, input.actions, output);
    pushResult(dataName.c_str(), libName.c_str(), output);

    cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd,
                                                           libName.c_str()};

    const auto report = gcov_measurer.getReport(output);
    if (report.has_value()) {
      pushResult(dataName.c_str(), input.testOrLibName.c_str(),
                 input.actions.size(), output.size(), report->report);
    }
  }

  return result;
}

QRandGenerationStage::QRandGenerationStage(
    const qleaning::GenerationSettings& settings,
    int numberOfRuns)
    : m_agent(qleaning::getAgent()),
      _settings(settings),
      _numberOfRuns(numberOfRuns) {}

bool QRandGenerationStage::process(const std::string& metadata,
                                   const std::string& libName,
                                   const cider::Cmd& cmd) {
  std::string prefix;
  getPrefix(_settings, prefix);

  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;
  outPath /= prefix;
  std::filesystem::create_directories(outPath);

  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
  measurer.setLogger(outPath.string(), "cfg_generation_log.txt");

  _settings.objFunc = measurer.getObjValueFunc();

  bool result = true;
  const auto dataName = std::string{"RAND"};
  const auto& input = getInput();

  for (int idx = 0; idx < _numberOfRuns; ++idx) {
    recorder::Actions output;
    result &= qlearningGenerationPipeline(m_agent, _settings, outPath.string(),
                                          libName, input.actions, output);
    pushResult(dataName.c_str(), libName.c_str(), output);

    cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd,
                                                           libName.c_str()};

    const auto report = gcov_measurer.getReport(output);
    if (report.has_value()) {
      pushResult(dataName.c_str(), libName.c_str(), input.actions.size(),
                 output.size(), report->report);
    }
  }

  return result;
}

bool generate(const qleaning::GenerationSettings& settings,
              const std::string& libName,
              const cider::Cmd& cmd,
              const recorder::Actions& input,
              recorder::Actions& output) {
  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};

  auto settings_ = settings;
  settings_.objFunc = measurer.getObjValueFunc();

  return gererationSession(settings_, qleaning::getAgent(), input, output);
}

}  // namespace pipelines
}  // namespace cider
