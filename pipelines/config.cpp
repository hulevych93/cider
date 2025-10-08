// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "pipes/aggregation-pipe.h"
#include "pipes/compression-pipe.h"
#include "pipes/data-pipe.h"
#include "pipes/eft-pipe.h"
#include "pipes/learning-pipe.h"
#include "pipes/verification-pipe.h"

#include "pipes/simulation/dslicer-pipe.h"
#include "pipes/simulation/greedy-r-pipe.h"
#include "pipes/simulation/mcts-pipe.h"
#include "pipes/simulation/meta-pipe.h"
#include "pipes/simulation/synthesis-pipe.h"

#include "pipes/graphics/cfg-heatmap-plot-report-pipe.h"
#include "pipes/graphics/compression-bar-plot-report-pipe.h"
#include "pipes/graphics/cov-box-plot-report-pipe.h"
#include "pipes/graphics/cov-grow-plot-report-pipe.h"
#include "pipes/graphics/efficiency-radar-plot-report-pipe.h"
#include "pipes/graphics/exec-times-bar-plot-report-pipe.h"
#include "pipes/graphics/graph-report-pipe.h"
#include "pipes/graphics/lines-bar-plot-report-pipe.h"
#include "pipes/graphics/times-bar-plot-report-pipe.h"

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

auto getDSLSettings() {
  dslicer::DSlicingSettings settings;
  settings.configName = "DSL";
  return settings;
}

auto getDSL_F_Settings() {
  dslicer::BatchDSlicingSettings settings;
  settings.configName = "DSL-F";
  settings.batchSize = 5;
  return settings;
}

auto getDSL_FM0_Settings() {
  dslicer::BatchMultiPassDSlicingSettings settings;
  settings.configName = "DSL-FM0";
  settings.initialStepRatio = 0.2;
  settings.minimalGranularity = 7;
  return settings;
}

auto getDSL_FM1_Settings() {
  dslicer::BatchMultiPassDSlicingSettings settings;
  settings.configName = "DSL-FM1";
  settings.initialStepRatio = 0.2;
  settings.minimalGranularity = 5;
  return settings;
}

auto getDSL_FM2_Settings() {
  dslicer::BatchMultiPassDSlicingSettings settings;
  settings.configName = "DSL-FM2";
  settings.initialStepRatio = 0.25;
  settings.minimalGranularity = 3;
  return settings;
}

auto getDSL_FM3_Settings() {
  dslicer::BatchMultiPassDSlicingSettings settings;
  settings.configName = "DSL-FM3";
  settings.initialStepRatio = 0.3;
  settings.minimalGranularity = 1;
  return settings;
}

auto getDSL_TR_Settings() {
  dslicer::BatchTracksDSlicingSettings settings;
  settings.configName = "DSL-TR";
  return settings;
}

auto getGreedySettings() {
  greedy_r::GreedyRSettings settings;
  settings.configName = "GR";

  settings.top_k = 1;
  settings.maxZeroGain = 0;
  return settings;
}

auto getGreedyR1Settings() {
  greedy_r::GreedyRSettings settings;
  settings.configName = "GRR1";

  settings.top_k = 3;
  settings.maxZeroGain = 5;
  return settings;
}

auto getGreedyR2Settings() {
  greedy_r::GreedyRSettings settings;
  settings.configName = "GRR2";

  settings.top_k = 5;
  settings.maxZeroGain = 5;
  return settings;
}

auto getGreedyR3Settings() {
  greedy_r::GreedyRSettings settings;
  settings.configName = "GRR3";

  settings.top_k = 7;
  settings.maxZeroGain = 5;
  return settings;
}

auto getGreedyRTracks1Settings() {
  greedy_r::GreedyRTracksSettings settings;
  settings.configName = "GRR-T1";

  settings.top_k = 5;
  settings.temperature = 0.5;
  return settings;
}

auto getGreedyRTracks2Settings() {
  greedy_r::GreedyRTracksSettings settings;
  settings.configName = "GRR-T2";

  settings.top_k = 5;
  settings.temperature = 1.5;
  return settings;
}

auto getGreedyRTracks3Settings() {
  greedy_r::GreedyRTracksSettings settings;
  settings.configName = "GRR-T3";

  settings.top_k = 7;
  settings.temperature = 2.0;
  return settings;
}

auto getGreedyRTracksD1Settings() {
  greedy_r::GreedyRTracksSettings settings;
  settings.configName = "GRR-TD1";

  settings.lambda = 0.35;
  settings.top_k = 5;
  settings.temperature = 1.5;
  return settings;
}

auto getGreedyRTracksD2Settings() {
  greedy_r::GreedyRTracksSettings settings;
  settings.configName = "GRR-TD2";

  settings.lambda = 0.65;
  settings.top_k = 5;
  settings.temperature = 1.5;
  return settings;
}

auto getGreedyRTracksD3Settings() {
  greedy_r::GreedyRTracksSettings settings;
  settings.configName = "GRR-TD3";

  settings.lambda = 0.9;
  settings.top_k = 5;
  settings.temperature = 1.5;
  return settings;
}

auto getMCTS1Settings() {
  mcts::MonteCarloSettings settings;
  settings.configName = "MCTS1";
  settings.maxIter = 100;
  settings.maxDepth = 500;
  settings.ucb_C = 0.7;
  settings.maxRollback = 10;
  return settings;
}

auto getMCTS2Settings() {
  mcts::MonteCarloSettings settings;
  settings.configName = "MCTS2";
  settings.maxIter = 50;
  settings.maxDepth = 800;
  settings.ucb_C = 1.4;
  settings.maxRollback = 100;
  return settings;
}

auto getMCTS3Settings() {
  mcts::MonteCarloSettings settings;
  settings.configName = "MCTS3";
  settings.maxIter = 100;
  settings.maxDepth = 500;
  settings.ucb_C = 2.0;
  settings.maxRollback = 10;
  return settings;
}

auto getQLearningSettings() {
  agent_model::qlearning::QLearningSettings settings;
  settings.configName = "QL";
  settings.prelearningEpisodes = 5U;
  settings.discountFactor = 0.85;
  settings.initialLearningRate = 0.3;
  settings.finalLearningRate = 0.1;
  settings.episodes = 1000U;
  settings.maxRollback = 20U;
  settings.coverageConvergenceCounter = 100U;
  settings.maxStateDepth = 5U;
  return settings;
}

auto getSarsaLearningSettings() {
  agent_model::sarsa::SarsaLearningSettings settings;
  settings.configName = "SL";
  settings.discountFactor = 0.85;
  settings.initialLearningRate = 0.1;
  settings.episodes = 1000;
  settings.maxRollback = 20U;
  settings.maxStateDepth = 5U;
  return settings;
}

auto getQGenG1Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QLEG1";
  settings.epsilon = 0.1;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getQGenG2Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QLEG2";
  settings.epsilon = 0.2;
  settings.maxRollback = 50U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getQGenG3Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QLEG3";
  settings.epsilon = 0.25;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenG1Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SLEG1";
  settings.epsilon = 0.1;
  settings.maxRollback = 10U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenG2Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SLEG2";
  settings.epsilon = 0.15;
  settings.maxRollback = 10U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenG3Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SLEG3";
  settings.epsilon = 0.25;
  settings.maxRollback = 10U;
  settings.strategy = synthesis::GenerationStrategyType::EGreedy;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getQGenB1Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QLB1";
  settings.temperature = 1.5;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getQGenB2Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QLB2";
  settings.temperature = 3.0;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getQGenB3Settings() {
  synthesis::QSynthesisSettings settings;
  settings.configName = "QLB3";
  settings.temperature = 5.0;
  settings.maxRollback = 30U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenB1Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SLB1";
  settings.temperature = 1.5;
  settings.maxRollback = 10U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenB2Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SLB2";
  settings.temperature = 3.0;
  settings.maxRollback = 10U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getSarsaGenB3Settings() {
  synthesis::SarsaSynthesisSettings settings;
  settings.configName = "SLB3";
  settings.temperature = 5.0;
  settings.maxRollback = 10U;
  settings.strategy = synthesis::GenerationStrategyType::Boltzmann;
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  return settings;
}

auto getRandGenSettings(size_t lines = 450) {
  synthesis::RandSynthesisSettings settings;
  settings.configName = "RAND";
  settings.stopType = synthesis::StopCondition::GreaterCoverage;
  settings.limitActions = lines;
  settings.maxRollback = 30;
  return settings;
}

const pipelines::ReportConfiguration& getReportConfigRAND() {
  static const std::vector<std::string> orderedMethods = {"RAND"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportConfigQLEG() {
  static const std::vector<std::string> orderedMethods = {"QLEG1", "QLEG2",
                                                          "QLEG3"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportConfigQLB() {
  static const std::vector<std::string> orderedMethods = {"RAND", "QLB1",
                                                          "QLB2", "QLB3"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportConfigSLEG() {
  static const std::vector<std::string> orderedMethods = {"RAND", "SLEG1",
                                                          "SLEG2", "SLEG3"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportConfigSLB() {
  static const std::vector<std::string> orderedMethods = {"RAND", "SLB1",
                                                          "SLB2", "SLB3"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportConfigMCTS() {
  static const std::vector<std::string> orderedMethods = {"MCTS1", "MCTS2",
                                                          "MCTS3"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportConfigQLEGvsQLB() {
  static const std::vector<std::string> orderedMethods = {
      "QLEG1", "QLEG2", "QLEG3", "QLB1", "QLB2", "QLB3"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportConfigGreedyR() {
  static const std::vector<std::string> orderedMethods = {"GRR-TD1", "GRR-TD2",
                                                          "GRR-TD3"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportConfigDSlicing() {
  static const std::vector<std::string> orderedMethods = {"DSL", "DSL-FM1",
                                                          "DSL-FM2", "DSL-FM3"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportConfigSelected() {
  static const std::vector<std::string> orderedMethods = {
      "DSL",     "GR",      "DSL-FM1", "DSL-FM2",
      "DSL-FM3", "GRR-TD1", "GRR-TD2", "GRR-TD3"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportConfigTarget() {
  static const std::vector<std::string> orderedMethods = {
      //"QLEG1", "QLEG2", "QLEG3", "QLB1", "QLB2", "QLB3"
      "QLB2+DSL"};
  return orderedMethods;
}

const pipelines::ReportConfiguration& getReportALLSelected() {
  static const std::vector<std::string> orderedMethods = {
      // ---- DD ----
      "DSL", "DSL-FM1", "DSL-FM2", "DSL-FM3",

      // ---- GREEDY ----
      "GR", "GRR1", "GRR2", "GRR3", "GRR-T1", "GRR-T2", "GRR-T3", "GRR-TD1",
      "GRR-TD2", "GRR-TD3",

      // --- MCTS ----
      "MCTS1", "MCTS2", "MCTS3",

      // --- QLEG1 ---
      "QLEG1", "QLEG1+DSL", "QLEG1+DSL-FM0", "QLEG1+DSL-FM1", "QLEG1+DSL-FM2",
      "QLEG1+DSL-FM3",

      // --- QLEG2 ---
      "QLEG2", "QLEG2+DSL", "QLEG2+DSL-FM0", "QLEG2+DSL-FM1", "QLEG2+DSL-FM2",
      "QLEG2+DSL-FM3",

      // --- QLEG3 ---
      "QLEG3", "QLEG3+DSL", "QLEG3+DSL-FM0", "QLEG3+DSL-FM1", "QLEG3+DSL-FM2",
      "QLEG3+DSL-FM3",

      // --- QLB1 ---
      "QLB1", "QLB1+DSL", "QLB1+DSL-FM0", "QLB1+DSL-FM1", "QLB1+DSL-FM2",
      "QLB1+DSL-FM3",

      // --- QLB2 ---
      "QLB2", "QLB2+DSL", "QLB2+DSL-FM0", "QLB2+DSL-FM1", "QLB2+DSL-FM2",
      "QLB2+DSL-FM3",

      // --- QLB3 ---
      "QLB3", "QLB3+DSL", "QLB3+DSL-FM0", "QLB3+DSL-FM1", "QLB3+DSL-FM2",
      "QLB3+DSL-FM3"};
  return orderedMethods;
}

constexpr const int StatsCount = 30U;
constexpr const int GreedyCount = 20U;
constexpr const int MCTSCount = 1U;

}  // namespace

namespace pipelines {

const pipelines::ReportConfiguration& getReportConfig(MethodsGroup group) {
  switch (group) {
    case MethodsGroup::RAND:
      return getReportConfigRAND();
    case MethodsGroup::QLEG:
      return getReportConfigQLEG();
    case MethodsGroup::QLB:
      return getReportConfigQLB();
    case MethodsGroup::SLEG:
      return getReportConfigSLEG();
    case MethodsGroup::SLB:
      return getReportConfigSLB();
    case MethodsGroup::MCTS:
      return getReportConfigMCTS();
    case MethodsGroup::QLEG2_VS_QLB2:
      return getReportConfigQLEGvsQLB();
    case MethodsGroup::GREEDY_R:
      return getReportConfigGreedyR();
    case MethodsGroup::DSL:
      return getReportConfigDSlicing();
    case MethodsGroup::SELECTED:
      return getReportConfigSelected();
    case MethodsGroup::TARGET:
      return getReportConfigTarget();
    case MethodsGroup::ALL:
      return getReportALLSelected();
  }
}

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
    case PipelineType::MCTS1:
      pipeline.addStage(
          std::make_unique<MctsSearchStage>(getMCTS1Settings(), MCTSCount));
      break;
    case PipelineType::MCTS2:
      pipeline.addStage(
          std::make_unique<MctsSearchStage>(getMCTS2Settings(), MCTSCount));
      break;
    case PipelineType::MCTS3:
      pipeline.addStage(
          std::make_unique<MctsSearchStage>(getMCTS3Settings(), MCTSCount));
      break;
    case PipelineType::Greedy:
      pipeline.addStage(
          std::make_unique<GreedyRStage>(getGreedySettings(), GreedyCount));
      break;
    case PipelineType::GreedyR1:
      pipeline.addStage(
          std::make_unique<GreedyRStage>(getGreedyR1Settings(), GreedyCount));
      break;
    case PipelineType::GreedyR2:
      pipeline.addStage(
          std::make_unique<GreedyRStage>(getGreedyR2Settings(), GreedyCount));
      break;
    case PipelineType::GreedyR3:
      pipeline.addStage(
          std::make_unique<GreedyRStage>(getGreedyR3Settings(), GreedyCount));
      break;
    case PipelineType::GreedyRTracks1:
      pipeline.addStage(std::make_unique<GreedyRStage>(
          getGreedyRTracks1Settings(), GreedyCount));
      break;
    case PipelineType::GreedyRTracks2:
      pipeline.addStage(std::make_unique<GreedyRStage>(
          getGreedyRTracks2Settings(), GreedyCount));
      break;
    case PipelineType::GreedyRTracks3:
      pipeline.addStage(std::make_unique<GreedyRStage>(
          getGreedyRTracks3Settings(), GreedyCount));
      break;
    case PipelineType::GreedyRTracksD1:
      pipeline.addStage(std::make_unique<GreedyRStage>(
          getGreedyRTracksD1Settings(), GreedyCount));
      break;
    case PipelineType::GreedyRTracksD2:
      pipeline.addStage(std::make_unique<GreedyRStage>(
          getGreedyRTracksD2Settings(), GreedyCount));
      break;
    case PipelineType::GreedyRTracksD3:
      pipeline.addStage(std::make_unique<GreedyRStage>(
          getGreedyRTracksD3Settings(), GreedyCount));
      break;
    case PipelineType::DSL:
      pipeline.addStage(std::make_unique<DSlicerStage>(getDSLSettings(), 5));
      break;
    case PipelineType::DSL_FM0:
      pipeline.addStage(
          std::make_unique<DSlicerStage>(getDSL_FM0_Settings(), 5));
      break;
    case PipelineType::DSL_FM1:
      pipeline.addStage(
          std::make_unique<DSlicerStage>(getDSL_FM1_Settings(), 5));
      break;
    case PipelineType::DSL_FM2:
      pipeline.addStage(
          std::make_unique<DSlicerStage>(getDSL_FM2_Settings(), 5));
      break;
    case PipelineType::DSL_FM3:
      pipeline.addStage(
          std::make_unique<DSlicerStage>(getDSL_FM3_Settings(), 5));
      break;
    case PipelineType::DSL_TR:
      pipeline.addStage(
          std::make_unique<DSlicerStage>(getDSL_TR_Settings(), 1));
      break;
    case PipelineType::DSL_PostProcessing:
      pipeline.addStage(std::make_unique<DQLPostProcessSlicerStage>(
          getDSLSettings(), getReportConfig(cmd.group)));
      break;
    case PipelineType::DSL_FM0_PostProcessing:
      pipeline.addStage(std::make_unique<DQLPostProcessSlicerStage>(
          getDSL_FM0_Settings(), getReportConfig(cmd.group)));
      break;
    case PipelineType::DSL_FM1_PostProcessing:
      pipeline.addStage(std::make_unique<DQLPostProcessSlicerStage>(
          getDSL_FM1_Settings(), getReportConfig(cmd.group)));
      break;
    case PipelineType::DSL_FM2_PostProcessing:
      pipeline.addStage(std::make_unique<DQLPostProcessSlicerStage>(
          getDSL_FM2_Settings(), getReportConfig(cmd.group)));
      break;
    case PipelineType::DSL_FM3_PostProcessing:
      pipeline.addStage(std::make_unique<DQLPostProcessSlicerStage>(
          getDSL_FM3_Settings(), getReportConfig(cmd.group)));
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
        pipeline.addStage(
            std::make_unique<BoxPlotReportStage>(getReportConfig(cmd.group)));
      }
      break;
    case PipelineType::GenerationCoverageStepperStats:
      pipeline.addStage(
          std::make_unique<StepperReportStage>(getReportConfig(cmd.group)));
      break;
    case PipelineType::GenerationLinesBarStats:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<LinesBarPlotReportStage>(
            getReportConfig(cmd.group)));
      }
      break;
    case PipelineType::GenerationTimesBarStats:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<TimesBarPlotReportStage>(
            getReportConfig(cmd.group)));
      }
      break;
    case PipelineType::GenerationExecTimesBarStats:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<ExecTimesBarPlotReportStage>(
            getReportConfig(cmd.group)));
      }
      break;
    case PipelineType::GenerationCoverageHeatMap:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<HeatmapPlotReportStage>(
            getReportConfig(cmd.group)));
      }
      break;
    case PipelineType::GenerationCompressionBarStats:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<CompressionBarPlotReportStage>(
            getReportConfig(cmd.group)));
      }
      break;
    case PipelineType::GenerationEfficencyTable:
      if (pipeline.hasResults()) {
        pipeline.addStage(
            std::make_unique<EfficencyReportStage>(getReportALLSelected()));
      }
      break;
    case PipelineType::GenerationEfficienctRadarPlotStats:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<EfficiencyRadarPlotReportStage>(
            getReportALLSelected()));
      }
      break;
    case PipelineType::AggregateData:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<ResultsAgregationStage>());
      }
      break;
    case PipelineType::VerifyData:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<ResultsVerificationStage>());
      }
      break;
    case PipelineType::ComputeCompression:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<ResultsCompressionStage>(
            getReportConfig(cmd.group)));
      }
      break;
    case PipelineType::RemoveGroupData:
      if (pipeline.hasResults()) {
        pipeline.addStage(
            std::make_unique<RemoveDataStage>(getReportConfig(cmd.group)));
      }
      break;
    case PipelineType::CleanupData:
      if (pipeline.hasResults()) {
        pipeline.addStage(
            std::make_unique<CleanUpDataStage>(getReportConfig(cmd.group)));
      }
      break;
    case PipelineType::ModifyData:
      if (pipeline.hasResults()) {
        pipeline.addStage(std::make_unique<ModifyDataStage>());
      }
      break;
    case PipelineType::QLearningStats:
      pipeline.addStage(std::make_unique<QLearningReportStage>());
      break;
    case PipelineType::ShowResults:
      pipeline.addStage(
          std::make_unique<ShowResultsStage>(getReportConfig(cmd.group)));
      break;
  }
  return pipeline;
}

}  // namespace pipelines
}  // namespace cider
