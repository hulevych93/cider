// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "dataset-pipe.h"
#include "meta-pipe.h"
#include "paths.h"
#include "q-learning-pipe.h"
#include "report-pipe.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cider {

namespace {

auto getHarmonySearchSettings() {
  metasearch::harmony::Settings settings;
  settings.mutationRate = 0.15;
  settings.harmonyMemoryConsiderationRate = 0.4;
  settings.harmonyMemorySize = 10;
  settings.maxIterationsWithoutUpdates = 50;
  settings.maxIter = 1000U;
  settings.strategy = cider::metasearch::MutationStrategy::ShuffleBytes;
  return settings;
}

auto getCackooSettings() {
  metasearch::cuckoo::Settings settings;
  settings.populationSize = 10;
  settings.Pa = 0.1;
  settings.maxIterationsWithoutUpdates = 50;
  settings.maxIter = 1000U;
  settings.strategy = cider::metasearch::MutationStrategy::LevyFlight;
  return settings;
}

auto getLearningSettings() {
  qleaning::LearningSettings settings;
  settings.discountFactor = 0.85;
  settings.learningRate = 0.1;
  settings.episodes = 2000U;
  settings.maxRollback = 20U;
  return settings;
}

auto getGenG1Settings() {
  qleaning::GenerationSettings settings;
  settings.configName = "G1";
  settings.epsilon = 0.1;
  settings.maxRollback = 30U;
  settings.strategy = qleaning::GenerationStrategyType::EGreedy;
  settings.stopType = qleaning::GenerationStopType::GreaterCoverage;
  return settings;
}

auto getGenG2Settings() {
  qleaning::GenerationSettings settings;
  settings.configName = "G2";
  settings.epsilon = 0.15;
  settings.maxRollback = 30U;
  settings.strategy = qleaning::GenerationStrategyType::EGreedy;
  settings.stopType = qleaning::GenerationStopType::GreaterCoverage;
  return settings;
}

auto getGenG3Settings() {
  qleaning::GenerationSettings settings;
  settings.configName = "G3";
  settings.epsilon = 0.25;
  settings.maxRollback = 30U;
  settings.strategy = qleaning::GenerationStrategyType::EGreedy;
  settings.stopType = qleaning::GenerationStopType::GreaterCoverage;
  return settings;
}

auto getGenB1Settings() {
  qleaning::GenerationSettings settings;
  settings.configName = "B1";
  settings.temperature = 1.5;
  settings.maxRollback = 30U;
  settings.strategy = qleaning::GenerationStrategyType::Boltzmann;
  settings.stopType = qleaning::GenerationStopType::GreaterCoverage;
  return settings;
}

auto getGenB2Settings() {
  qleaning::GenerationSettings settings;
  settings.configName = "B2";
  settings.temperature = 3.0;
  settings.maxRollback = 30U;
  settings.strategy = qleaning::GenerationStrategyType::Boltzmann;
  settings.stopType = qleaning::GenerationStopType::GreaterCoverage;
  return settings;
}

auto getGenB3Settings() {
  qleaning::GenerationSettings settings;
  settings.configName = "B3";
  settings.temperature = 5.0;
  settings.maxRollback = 30U;
  settings.strategy = qleaning::GenerationStrategyType::Boltzmann;
  settings.stopType = qleaning::GenerationStopType::GreaterCoverage;
  return settings;
}

auto getRandSettings(size_t lines = 250) {
  qleaning::GenerationSettings settings;
  settings.strategy = qleaning::GenerationStrategyType::Random;
  settings.stopType = qleaning::GenerationStopType::LimitActions;
  settings.limitActions = lines;
  return settings;
}

constexpr const int StatsCount = 50U;

}  // namespace

namespace pipelines {

Pipeline makePipeline(const std::string& libName, const cider::Cmd& cmd) {
  Pipeline pipeline(libName, cmd);
  const auto pipelineType = cmd.pipelineType;
  pipeline.setType(pipelineType);

  switch (pipelineType) {
    case PipelineType::HarmonySearch:
      pipeline.addStage(
          std::make_unique<HarmonySearchStage>(getHarmonySearchSettings()));
      break;
    case PipelineType::CackooSearch:
      pipeline.addStage(
          std::make_unique<CackooSearchStage>(getCackooSettings()));
      break;
    case PipelineType::QLearningAgent:
      pipeline.addStage(
          std::make_unique<QPreLearningStage>(getLearningSettings()));
      pipeline.addStage(
          std::make_unique<QLearningStage>(getLearningSettings()));
      break;
    case PipelineType::QLearningAgentGenerationStepper:
      if (!pipeline.hasBrieft()) {
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenG2Settings(), StatsCount));
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenB2Settings(), StatsCount));
        pipeline.addStage(std::make_unique<QRandGenerationStage>(
            getRandSettings(), StatsCount));
      }

      pipeline.addStage(std::make_unique<StepperReportStage>());
      break;
    case PipelineType::QLearningAgentGenerationGreedyBoxStats:
      if (!pipeline.hasBrieft()) {
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenG1Settings(), StatsCount));
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenG2Settings(), StatsCount));
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenG3Settings(), StatsCount));
        pipeline.addStage(std::make_unique<QRandGenerationStage>(
            getRandSettings(), StatsCount));
      }
      pipeline.addStage(std::make_unique<BoxPlotReportStage>());
      break;
    case PipelineType::QLearningAgentGenerationBolzmanBoxStats:
      if (!pipeline.hasBrieft()) {
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenB1Settings(), StatsCount));
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenB2Settings(), StatsCount));
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenB3Settings(), StatsCount));
        pipeline.addStage(std::make_unique<QRandGenerationStage>(
            getRandSettings(), StatsCount));
      }
      pipeline.addStage(std::make_unique<BoxPlotReportStage>());
      break;
    case PipelineType::QLearningAgentGenerationBolzmanGreedyBoxStats:
      if (!pipeline.hasBrieft()) {
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenG2Settings(), StatsCount));
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenB2Settings(), StatsCount));
        pipeline.addStage(std::make_unique<QRandGenerationStage>(
            getRandSettings(), StatsCount));
      }
      pipeline.addStage(std::make_unique<BoxPlotReportStage>());
      break;
    case PipelineType::QLearningAgentGenerationLineBoxStats:
      if (!pipeline.hasBrieft()) {
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenG2Settings(), StatsCount));
        pipeline.addStage(
            std::make_unique<QGenerationStage>(getGenB2Settings(), StatsCount));
      }
      pipeline.addStage(std::make_unique<LinesBarPlotReportStage>());
      break;
    case PipelineType::DataSetPlot:
      pipeline.addStage(std::make_unique<DatasetStage>());
      pipeline.addStage(std::make_unique<CovBarPlotReportStage>());
      break;
    default:
      break;
  }
  return pipeline;
}

}  // namespace pipelines
}  // namespace cider
