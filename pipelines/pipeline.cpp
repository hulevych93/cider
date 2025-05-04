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
    : _libName(libName), _cmd(cmd), _report(std::make_unique<ReportStage>()) {}

Pipeline makePipeline(const std::string& libName, const cider::Cmd& cmd) {
  Pipeline pipeline(libName, cmd);
  const auto pipelineType = cmd.pipelineType;
  switch (pipelineType) {
    case PipelineType::HarmonySearch:
      pipeline.addStage(std::make_unique<HarmonySearchStage>());
      break;
    case PipelineType::CackooSearch:
      pipeline.addStage(std::make_unique<CackooSearchStage>());
      break;
    case PipelineType::QLearningAgent:
      pipeline.addStage(std::make_unique<QLearningStage>());
      break;
    case PipelineType::QLearningAgentPlusHarmonySearch:
      pipeline.addStage(std::make_unique<QLearningStage>());
      pipeline.addStage(std::make_unique<HarmonySearchStage>());
      break;
    case PipelineType::QLearningAgentPlusCackooSearch:
      pipeline.addStage(std::make_unique<QLearningStage>());
      pipeline.addStage(std::make_unique<CackooSearchStage>());
      break;
    default:
      break;
  }
  return pipeline;
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
