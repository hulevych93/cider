// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "pipes/dataset-pipe.h"
#include "pipes/learning-pipe.h"
#include "pipes/mcts-pipe.h"
#include "pipes/meta-pipe.h"
#include "pipes/report-pipe.h"
#include "pipes/synthesis-pipe.h"

#include "agent-q-learning/q-learning-agent.h"
#include "agent-sarsa-learning/sarsa-learning-agent.h"

#include "paths.h"

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

auto getHS3Settings() {
  metasearch::harmony_synthesis::Settings settings;
  settings.configName = "HS3";
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

auto getMCTS0Settings() {
  mcts::MonteCarloSettings settings;
  settings.configName = "MCTS0";
  settings.maxIter = 200;
  settings.maxDepth = 25;
  settings.ucb_C = 1.4;
  settings.maxRollback = 20;
  return settings;
}

auto getQLearningSettings() {
  agent_model::qlearning::QLearningSettings settings;
  settings.configName = "QL";
  settings.discountFactor = 0.85;
  settings.learningRate = 0.1;
  settings.episodes = 2000U;
  settings.maxRollback = 20U;
  settings.maxStateDepth = 10U;
  return settings;
}

auto getSarsaLearningSettings() {
  agent_model::sarsa::SarsaLearningSettings settings;
  settings.configName = "SARSA";
  settings.discountFactor = 0.85;
  settings.learningRate = 0.1;
  settings.episodes = 1000;
  settings.maxRollback = 20U;
  settings.maxStateDepth = 5U;
  return settings;
}

auto getQGenG1Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QEG1";
  settings.epsilon = 0.1;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getQGenG2Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QEG2";
  settings.epsilon = 0.15;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getQGenG3Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QEG3";
  settings.epsilon = 0.25;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenG1Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SEG1";
  settings.epsilon = 0.1;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenG2Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SEG2";
  settings.epsilon = 0.15;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenG3Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SEG3";
  settings.epsilon = 0.25;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getQGenB1Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QB1";
  settings.temperature = 1.5;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getQGenB2Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QB2";
  settings.temperature = 3.0;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getQGenB3Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QB3";
  settings.temperature = 5.0;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenB1Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SB1";
  settings.temperature = 1.5;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenB2Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SB2";
  settings.temperature = 3.0;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenB3Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SB3";
  settings.temperature = 5.0;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getRandGenSettings(size_t lines = 250) {
  synthesis::RandSynthesisSettings settings;
  settings.configName = "GRAND";
  settings.stopType = synthesis::StopCondition::LimitActions;
  settings.limitActions = lines;
  return settings;
}

constexpr const int StatsCount = 20U;

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
    case PipelineType::HS3:
      pipeline.addStage(
          std::make_unique<MetaSearchStage>(getHS3Settings(), StatsCount));
      break;
    case PipelineType::CackooSearch:
      pipeline.addStage(
          std::make_unique<MetaSearchStage>(getCackooSettings(), StatsCount));
      break;
    case PipelineType::GRAND:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getRandGenSettings(), StatsCount));
      break;
    case PipelineType::MCTS0:
      pipeline.addStage(
          std::make_unique<MctsSearchStage>(getMCTS0Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentLearning:
      if (!agent_model::qlearning::QLearningAgent::get().isLoaded()) {
        pipeline.addStage(
            std::make_unique<PreLearningStage>(getQLearningSettings()));
      }
      pipeline.addStage(
          std::make_unique<LearningStage>(getQLearningSettings()));
      break;
    case PipelineType::SarsaAgentLearning:
      if (!agent_model::sarsa::SarsaLearningAgent::get().isLoaded()) {
        pipeline.addStage(
            std::make_unique<PreLearningStage>(getSarsaLearningSettings()));
      }
      pipeline.addStage(
          std::make_unique<LearningStage>(getSarsaLearningSettings()));
      break;
    case PipelineType::QLearningAgentG1:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getQGenG1Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentG2:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getQGenG2Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentG3:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getQGenG3Settings(), StatsCount));
      break;
    case PipelineType::SarsaAgentG1:
      pipeline.addStage(std::make_unique<SynthesisStage>(
          getSarsaGenG1Settings(), StatsCount));
      break;
    case PipelineType::SarsaAgentG2:
      pipeline.addStage(std::make_unique<SynthesisStage>(
          getSarsaGenG2Settings(), StatsCount));
      break;
    case PipelineType::SarsaAgentG3:
      pipeline.addStage(std::make_unique<SynthesisStage>(
          getSarsaGenG3Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentB1:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getQGenB1Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentB2:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getQGenB2Settings(), StatsCount));
      break;
    case PipelineType::QLearningAgentB3:
      pipeline.addStage(
          std::make_unique<SynthesisStage>(getQGenB3Settings(), StatsCount));
      break;
    case PipelineType::SarsaAgentB1:
      pipeline.addStage(std::make_unique<SynthesisStage>(
          getSarsaGenB1Settings(), StatsCount));
      break;
    case PipelineType::SarsaAgentB2:
      pipeline.addStage(std::make_unique<SynthesisStage>(
          getSarsaGenB2Settings(), StatsCount));
      break;
    case PipelineType::SarsaAgentB3:
      pipeline.addStage(std::make_unique<SynthesisStage>(
          getSarsaGenB3Settings(), StatsCount));
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
  }
  return pipeline;
}

}  // namespace pipelines
}  // namespace cider
