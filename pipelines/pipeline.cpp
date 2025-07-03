// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "coverage/cfg_measurer.h"
#include "paths.h"
#include "q-learning/q-learning.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cider {
namespace pipelines {

namespace {

auto getGenSettings() {
  qleaning::GenerationSettings settings;
  settings.configName = "G2";
  settings.epsilon = 0.15;
  settings.maxRollback = 30U;
  settings.strategy = qleaning::GenerationStrategyType::EGreedy;
  settings.stopType = qleaning::GenerationStopType::GreaterCoverage;
  return settings;
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

}  // namespace

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

bool Pipeline::loadBrief(const std::string& filePath) {
  try {
    serialization::Deserializer deserializer(filePath);
    deserializer >> _briefResultsStorage;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool Pipeline::saveBrief(const std::string& filePath) const {
  try {
    serialization::Serializer serializer;
    serializer << _briefResultsStorage;
    serializer.save(filePath);
  } catch (...) {
    return false;
  }
  return true;
}

bool Pipeline::loadSessions(const std::string& filePath) {
  try {
    serialization::Deserializer deserializer(filePath);
    deserializer >> _sessionsResultsStorage;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool Pipeline::saveSessions(const std::string& filePath) const {
  try {
    serialization::Serializer serializer;
    serializer << _sessionsResultsStorage;
    serializer.save(filePath);
  } catch (...) {
    return false;
  }
  return true;
}

void Pipe::pushResult(const char* name,
                      const std::vector<recorder::Actions>& sessions) {
  SessionsResult result;
  result.sessions = sessions;
  result.libName = name;

  _owner->getSessionsResults().emplace_back(std::move(result));
}

void Pipe::pushResult(const char* methodName,
                      const char* libOrTestName,
                      const recorder::Actions& actions) {
  Result result;
  result.actions = deepCopy(actions);
  result.methodName = methodName;
  result.testOrLibName = libOrTestName;

  _owner->getResults().emplace_back(std::move(result));
}

void Pipe::pushResult(const char* methodName,
                      const char* libOrTestName,
                      int oldLines,
                      int newLines,
                      const gcov_coverage::CoverageReport& report) {
  BriefResult result;
  result.report = report;
  result.oldLines = oldLines;
  result.newLines = newLines;
  result.methodName = methodName;
  result.testOrLibName = libOrTestName;

  _owner->getBriefResults().emplace_back(std::move(result));
}

const Input& Pipe::getInput() const {
  return _owner->_input;
}

const Results& Pipe::getResults() const {
  return _owner->getResults();
}

const BriefResults& Pipe::getBriefResults() const {
  return _owner->getBriefResults();
}

Pipeline::Pipeline(const std::string& libName, const cider::Cmd& cmd)
    : _libName(libName), _cmd(cmd) {
  qleaning::getAgent(paths::getQTableAgentPath(cmd.resultsDir));

  std::cout << "Load results: " << paths::getResultsPath(cmd.resultsDir)
            << ", status: " << load(paths::getResultsPath(cmd.resultsDir))
            << std::endl;

  std::cout << "Load brief results: "
            << paths::getBriefResultsPath(cmd.resultsDir) << ", status: "
            << loadBrief(paths::getBriefResultsPath(cmd.resultsDir))
            << std::endl;

  std::cout << "Load sessions results: "
            << paths::getSessionsResultsPath(cmd.resultsDir) << ", status: "
            << loadSessions(paths::getSessionsResultsPath(cmd.resultsDir))
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

    const auto& agent = qleaning::getAgent();
    if (agent.isLoaded()) {
      _input.actionsCallback = [this]() {
        recorder::Actions output;
        generate(getGenSettings(), _libName, _cmd, _input.actions, output);
        return output;
      };
    }

    if (!run(metadata)) {
      std::cout << "pipeline failed." << std::endl;
    }

    // getResults().clear();
    // getBriefResults().clear();

    ++scrNum;
  }

  std::cout << "Save results: " << paths::getResultsPath(_cmd.resultsDir)
            << ", status: " << save(paths::getResultsPath(_cmd.resultsDir))
            << std::endl;

  std::cout << "Save brief results: "
            << paths::getBriefResultsPath(_cmd.resultsDir) << ", status: "
            << saveBrief(paths::getBriefResultsPath(_cmd.resultsDir))
            << std::endl;

  return true;
}

bool Pipeline::runAll(
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions) {
  const auto dateTime = getDatetimeForDirName();
  const auto config = pipelineConfig();

  auto scrNum = 0;
  SessionsResult result;
  for (const auto& session : sessions) {
    std::cout << "num: " << scrNum++ << "\t name: " << session->getName()
              << "\t count op: " << session->getInstructionsCount()
              << std::endl;
    result.sessions.emplace_back(session->getInstructions());
  }

  const auto metadata =
      dateTime + '_' + config + '/' + _libName + '_' + std::to_string(scrNum);

  result.libName = "Original";
  getSessionsResults().emplace_back(std::move(result));

  if (!run(metadata)) {
    std::cout << "pipeline failed." << std::endl;
  }

  std::cout << "Save sessions results: "
            << paths::getSessionsResultsPath(_cmd.resultsDir) << ", status: "
            << saveSessions(paths::getSessionsResultsPath(_cmd.resultsDir))
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
