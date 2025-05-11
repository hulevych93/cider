// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "meta-pipe.h"
#include "q-learning-pipe.h"
#include "report-pipe.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cider {
namespace pipelines {

Pipeline::Pipeline(const std::string& libName, const cider::Cmd& cmd)
    : _libName(libName), _cmd(cmd) {
  qleaning::QValuesAgent::getInstance(cmd.resultsDir + "/agent.img");
}

bool Pipeline::run(
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions) {
  const auto dateTime = getDatetimeForDirName();
  const auto config = pipelineConfig();
  int scrNum = 0;
  for (const auto& session : sessions) {
    std::cout << "Session: " << scrNum
                << ", name: " << session->getName() << ", instr: " << session->getInstructionsCount() << std::endl;
    const auto metadata =
        dateTime + '_' + config + '/' + session->getName() + '_' + std::to_string(scrNum);
    if (!run(metadata, session->getInstructions())) {
      return false;
    }
    ++scrNum;
  }

  return true;
}

bool Pipeline::run(const std::string& metadata, const Actions& input) {
  try {
    Actions in = deepCopy(input);
    Actions out;
    for (auto& pipe : _pipes) {
      if (!pipe->process(metadata, _libName, _cmd, in, out)) {
        return false;
      }
      in = deepCopy(out);
    }
    if (_report) {
      _report->process(metadata, _libName, _cmd, input, out);
    }
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

void Pipeline::enableReport() {
  _report = std::make_unique<ReportStage>();
}

Pipeline makePipeline(const std::string& libName, const cider::Cmd& cmd) {
  Pipeline pipeline(libName, cmd);
  const auto pipelineType = cmd.pipelineType;
  switch (pipelineType) {
    case PipelineType::HarmonySearch:
      pipeline.addStage(std::make_unique<HarmonySearchStage>());
      pipeline.enableReport();
      break;
    case PipelineType::CackooSearch:
      pipeline.addStage(std::make_unique<CackooSearchStage>());
      pipeline.enableReport();
      break;
    case PipelineType::QLearningAgent:
      pipeline.addStage(std::make_unique<QPreLearningStage>());
      pipeline.addStage(std::make_unique<QLearningStage>());
      break;
    case PipelineType::QLearningAgentPlusHarmonySearch:
      pipeline.addStage(std::make_unique<QGenerationStage>());
      pipeline.addStage(std::make_unique<HarmonySearchStage>());
      pipeline.enableReport();
      break;
    case PipelineType::QLearningAgentPlusCackooSearch:
      pipeline.addStage(std::make_unique<QGenerationStage>());
      pipeline.addStage(std::make_unique<CackooSearchStage>());
      pipeline.enableReport();
      break;
    case PipelineType::QLearningAgentGeneration:
      pipeline.addStage(std::make_unique<QGenerationStage>());
      pipeline.enableReport();
      break;
    default:
      break;
  }
  return pipeline;
}

}  // namespace pipelines
}  // namespace cider
