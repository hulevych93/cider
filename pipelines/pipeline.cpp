// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "agent-q-learning/q-learning-agent.h"
#include "agent-sarsa-learning/sarsa-learning-agent.h"

#include "paths.h"

#include <chrono>
#include <ctime>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace cider {
namespace pipelines {

bool Pipeline::load(const std::string& filePath) {
  try {
    serialization::Deserializer deserializer(filePath);
    deserializer >> _results;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool Pipeline::save(const std::string& filePath) const {
  try {
    serialization::Serializer serializer;
    serializer << _results;
    serializer.save(filePath);
  } catch (...) {
    return false;
  }
  return true;
}

Pipeline::Pipeline(const std::string& libName, const cider::Cmd& cmd)
    : _libName(libName), _cmd(cmd) {
  agent_model::qlearning::QLearningAgent::setPath(
      paths::getQLearningAgentPath(cmd.resultsDir));

  agent_model::sarsa::SarsaLearningAgent::setPath(
      paths::getSarsaAgentPath(cmd.resultsDir));

  std::cout << "Load results: " << paths::getResultsPath(cmd.resultsDir)
            << ", status: " << load(paths::getResultsPath(cmd.resultsDir))
            << std::endl;
}

bool Pipeline::runOneByOne(
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions) {
  const auto dateTime = getDatetimeForDirName();
  const auto config = pipelineConfig();

  auto scrNum = 0;
  for (const auto& session : sessions) {
    std::cout << "num: " << scrNum++ << "\t name: " << session->getName()
              << "\t count op: " << session->getInstructionsCount()
              << std::endl;
  }
  scrNum = 0;
  for (const auto& session : sessions) {
    std::cout << "num: " << scrNum << "\t name: " << session->getName()
              << "\t count op: " << session->getInstructionsCount()
              << std::endl;
    const auto metadata = dateTime + '_' + config + '/' + session->getName() +
                          '_' + std::to_string(scrNum);

    _input.actions = session->getInstructions();
    _input.testOrLibName = session->getName();

    if (!run(metadata)) {
      std::cout << "pipeline failed." << std::endl;
    }

    ++scrNum;
  }

  clearTrash();
  printResultsSummary(_results);

  return true;
}

bool Pipeline::save() {
  const auto& resultsDir = paths::getResultsPath(_cmd.resultsDir);
  std::cout << "Save results: " << resultsDir
            << ", status: " << save(resultsDir) << std::endl;
  return true;
}

bool Pipeline::run(const std::string& metadata) {
  for (auto& pipe : _pipes) {
    try {
      if (!pipe->process(metadata, _libName, _cmd)) {
        return false;
      }
    } catch (...) {
    }
  }
  return true;
}

void Pipeline::clearTrash() {
  for (auto& storageIt : _results) {
    auto& results = storageIt.second;
    results.erase(std::remove_if(results.begin(), results.end(),
                                 [](const Result& r) {
                                   return r.oldReport.branchCov.percent <=
                                              3.0 ||
                                          r.oldCfgReport.getPercentage() <= 3.0;
                                 }),
                  results.end());
  }
}

const std::string Pipeline::pipelineConfig() const {
  std::string config;
  for (const auto& pipe : _pipes) {
    config += pipe->getLetter() + '_';
  }
  config.pop_back();
  return config;
}

std::string Pipeline::getDatetimeForDirName() {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm = *std::localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d");
  return oss.str();
}

}  // namespace pipelines
}  // namespace cider
