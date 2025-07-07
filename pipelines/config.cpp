// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "pipes/dataset-pipe.h"
#include "pipes/meta-pipe.h"
#include "pipes/q-learning-pipe.h"
#include "pipes/report-pipe.h"
#include "pipes/synthesis-pipe.h"

#include "paths.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cider {

namespace {

auto getHS0Settings() {
  metasearch::harmony::Settings settings;
  settings.configName = "HS0";
  settings.mutationRate = 0.15;
  settings.harmonyMemoryConsiderationRate = 0.4;
  settings.harmonyMemorySize = 10;
  settings.maxIterationsWithoutUpdates = 50;
  settings.maxIter = 1000U;
  settings.strategy = cider::metasearch::ArgsMutationStrategy::LevyFlight;
  settings.instructionsMutationStrategy =
      cider::metasearch::InstructionsMutationStrategy::None;
  return settings;
}

auto getHS1Settings() {
  metasearch::harmony::Settings settings;
  settings.configName = "HS1";
  settings.mutationRate = 0.15;
  settings.harmonyMemoryConsiderationRate = 0.4;
  settings.harmonyMemorySize = 10;
  settings.maxIterationsWithoutUpdates = 300;
  settings.maxIter = 1000U;
  settings.strategy = cider::metasearch::ArgsMutationStrategy::None;
  settings.instructionsMutationStrategy =
      cider::metasearch::InstructionsMutationStrategy::Shuffle;
  return settings;
}

auto getHS2Settings() {
  metasearch::harmony::Settings settings;
  settings.configName = "HS2";
  settings.mutationRate = 0.15;
  settings.harmonyMemoryConsiderationRate = 0.4;
  settings.harmonyMemorySize = 10;
  settings.maxIterationsWithoutUpdates = 300;
  settings.maxIter = 1000U;
  settings.strategy = cider::metasearch::ArgsMutationStrategy::LevyFlight;
  settings.instructionsMutationStrategy =
      cider::metasearch::InstructionsMutationStrategy::Shuffle;
  return settings;
}

auto getCackooSettings() {
  metasearch::cuckoo::Settings settings;
  settings.configName = "CS0";
  settings.populationSize = 10;
  settings.Pa = 0.1;
  settings.maxIterationsWithoutUpdates = 50;
  settings.maxIter = 1000U;
  settings.strategy = cider::metasearch::ArgsMutationStrategy::LevyFlight;
  return settings;
}

auto getLearningSettings() {
  qleaning::LearningSettings settings;
  settings.discountFactor = 0.85;
  settings.learningRate = 0.1;
  settings.episodes = 2000U;
  settings.maxRollback = 20U;
  settings.maxStateDepth = 10U;
  return settings;
}

auto getGenG1Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "G1";
  settings.epsilon = 0.1;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getGenG2Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "G2";
  settings.epsilon = 0.15;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getGenG3Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "G3";
  settings.epsilon = 0.25;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getGenB1Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "B1";
  settings.temperature = 1.5;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getGenB2Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "B2";
  settings.temperature = 3.0;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getGenB3Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "B3";
  settings.temperature = 5.0;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getRandSettings(size_t lines = 250) {
  synthesis::RandSynthesisSettings settings;
  settings.configName = "GRAND";
  settings.stopType = synthesis::StopCondition::LimitActions;
  settings.limitActions = lines;
  return settings;
}

constexpr const int StatsCount = 50U;

}  // namespace

namespace pipelines {

Pipeline makePipeline(const std::string& libName, const cider::Cmd& cmd) {
  Pipeline pipeline(libName, cmd);
  const auto pipelineType = cmd.pipelineType;

  switch (pipelineType) {
    case PipelineType::HS0:
      pipeline.addStage(
          std::make_unique<MetaSearchStage>(getHS0Settings(), StatsCount));
      break;
    case PipelineType::HS1:
      pipeline.addStage(
          std::make_unique<MetaSearchStage>(getHS1Settings(), StatsCount));
      break;
    case PipelineType::HS2:
      pipeline.addStage(
          std::make_unique<MetaSearchStage>(getHS2Settings(), StatsCount));
      break;
    case PipelineType::CackooSearch:
      pipeline.addStage(
          std::make_unique<MetaSearchStage>(getCackooSettings(), StatsCount));
      break;
    case PipelineType::GRAND:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getRandSettings(), StatsCount));
      break;
    case PipelineType::QLearningAgentLearning:
      pipeline.addStage(
          std::make_unique<QPreLearningStage>(getLearningSettings()));
      pipeline.addStage(
          std::make_unique<QLearningStage>(getLearningSettings()));
      break;
    case PipelineType::QLearningAgentG1:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getGenG1Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentG2:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getGenG2Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentG3:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getGenG3Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentB1:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getGenB1Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentB2:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getGenB2Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentB3:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getGenB3Settings(), StatsCount));
      break;
    case PipelineType::GenerationCoverageBoxStats:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<BoxPlotReportStage>());
      }
      break;
    case PipelineType::GenerationCoverageStepperStats:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<StepperReportStage>());
      }
      break;
    case PipelineType::GenerationLinesBarStats:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<LinesBarPlotReportStage>());
      }
      break;
    default:
      break;
  }
  return pipeline;
}

}  // namespace pipelines
}  // namespace cider
