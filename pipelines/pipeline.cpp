// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "meta-pipe.h"
#include "paths.h"
#include "q-learning-pipe.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cider {
namespace pipelines {

void Pipe::pushResult(const Actions& result) {
  _owner->_results.emplace_back(result);
}

const Results& Pipe::getResults() const {
  return _owner->_results;
}

Pipeline::Pipeline(const std::string& libName, const cider::Cmd& cmd)
    : _libName(libName), _cmd(cmd) {
  qleaning::getAgent(paths::getQTableAgentPath(cmd.resultsDir));
}

bool Pipeline::run(
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions) {
  const auto dateTime = getDatetimeForDirName();
  const auto config = pipelineConfig();

  for (const auto& session : sessions) {
    std::cout << "name: " << session->getName()
              << "\t count op: " << session->getInstructionsCount()
              << std::endl;
  }

  int scrNum = 0;
  for (const auto& session : sessions) {
    std::cout << "num: " << scrNum << "\t name: " << session->getName()
              << "\t count op: " << session->getInstructionsCount()
              << std::endl;
    const auto metadata = dateTime + '_' + config + '/' + session->getName() +
                          '_' + std::to_string(scrNum);
    if (!run(metadata, session->getInstructions())) {
      return false;
    }
    ++scrNum;
  }

  return true;
}

bool Pipeline::run(const std::string& metadata, const Actions& input) {
  try {
    const Actions in = deepCopy(input);
    for (auto& pipe : _pipes) {
      if (!pipe->process(metadata, _libName, _cmd, in)) {
        return false;
      }
    }
    _results.clear();
    return true;
  } catch (...) {
  }
  return false;
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

Pipeline makePipeline(const std::string& libName, const cider::Cmd& cmd) {
  Pipeline pipeline(libName, cmd);
  const auto pipelineType = cmd.pipelineType;
  switch (pipelineType) {
    case PipelineType::HarmonySearch:
      pipeline.addStage(std::make_unique<HarmonySearchStage>());
      pipeline.addStage(std::make_unique<MetaReportStage>());
      break;
    case PipelineType::CackooSearch:
      pipeline.addStage(std::make_unique<CackooSearchStage>());
      pipeline.addStage(std::make_unique<MetaReportStage>());
      break;
    case PipelineType::QLearningAgent:
      pipeline.addStage(std::make_unique<QPreLearningStage>());
      pipeline.addStage(std::make_unique<QLearningStage>());
      break;
    case PipelineType::QLearningAgentPlusHarmonySearch:
      pipeline.addStage(std::make_unique<QGenerationStage>());
      pipeline.addStage(std::make_unique<HarmonySearchStage>());
      break;
    case PipelineType::QLearningAgentPlusCackooSearch:
      pipeline.addStage(std::make_unique<QGenerationStage>());
      pipeline.addStage(std::make_unique<CackooSearchStage>());
      break;
    case PipelineType::QLearningAgentGeneration:
      pipeline.addStage(std::make_unique<QGenerationStage>());
      pipeline.addStage(std::make_unique<QRandGenerationStage>());
      pipeline.addStage(std::make_unique<GenerationReportStage>());
      break;
    default:
      break;
  }
  return pipeline;
}

}  // namespace pipelines
}  // namespace cider
