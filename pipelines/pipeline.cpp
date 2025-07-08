// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "agent-q-learning/q-learning-agent.h"
#include "agent-sarsa-learning/sarsa-learning-agent.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

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
    deserializer >> _resultsStorage;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool Pipeline::save(const std::string& filePath) const {
  try {
    serialization::Serializer serializer;
    serializer << _resultsStorage;
    serializer.save(filePath);
  } catch (...) {
    return false;
  }
  return true;
}

void Pipe::pushResult(const std::string& libName,
                      const cider::Cmd& cmd,
                      const std::string& methodName,
                      Result result) {
  cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd, libName.c_str()};
  const auto oldGcovReport = gcov_measurer.getReport(result.oldActions);
  const auto newGcovReport = gcov_measurer.getReport(result.newActions);
  if (oldGcovReport.has_value() && newGcovReport.has_value()) {
    result.oldReport = oldGcovReport->report;
    result.newReport = newGcovReport->report;
  }

  cider::cfg_coverage::CoverageMeasurment cgf_measurer{cmd, libName.c_str()};
  const auto oldCfgReport = cgf_measurer.getReport(result.oldActions);
  const auto newCfgReport = cgf_measurer.getReport(result.newActions);
  if (oldCfgReport.has_value() && newCfgReport.has_value()) {
    result.oldCfgReport = oldCfgReport.value();
    result.newCgfReport = newCfgReport.value();
  }

  _owner->getResults()[methodName].emplace_back(std::move(result));
}

const Input& Pipe::getInput() const {
  return _owner->_input;
}

const Results& Pipe::getResults() const {
  return _owner->getResults();
}

Pipeline::Pipeline(const std::string& libName, const cider::Cmd& cmd)
    : _libName(libName), _cmd(cmd) {
  agent_model::qlearning::QLearningAgent::get(
      paths::getQLearningAgentPath(cmd.resultsDir));

  agent_model::sarsa::SarsaLearningAgent::get(
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

  std::cout << "Save results: " << paths::getResultsPath(_cmd.resultsDir)
            << ", status: " << save(paths::getResultsPath(_cmd.resultsDir))
            << std::endl;

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
  oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
  return oss.str();
}

}  // namespace pipelines
}  // namespace cider
