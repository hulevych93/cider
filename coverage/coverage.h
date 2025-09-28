// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <random>
#include <string>
#include <thread>

namespace cider {

enum class PipelineType {
  HS0 = 0,
  HS1 = 1,
  HS2 = 2,
  HS3 = 3,

  CackooSearch = 10,

  GRAND = 20,

  MCTS1 = 30,
  MCTS2 = 31,
  MCTS3 = 32,

  Greedy = 39,
  GreedyR1 = 40,
  GreedyR2 = 41,
  GreedyR3 = 42,

  GreedyRTracks1 = 200,
  GreedyRTracks2 = 201,
  GreedyRTracks3 = 202,

  DSL_FM0 = 34,
  DSL_FM1 = 35,
  DSL_FM2 = 36,
  DSL_FM3 = 37,

  DSL = 44,

  DSL_TR = 15,

  DSL_PostProcessing = 45,
  DSL_FM0_PostProcessing = 46,
  DSL_FM1_PostProcessing = 47,
  DSL_FM2_PostProcessing = 48,
  DSL_FM3_PostProcessing = 49,

  QLearningAgentLearning = 50,
  SarsaAgentLearning = 51,

  QLearningAgentG1 = 60,
  QLearningAgentG2 = 61,
  QLearningAgentG3 = 62,

  SarsaAgentG1 = 65,
  SarsaAgentG2 = 66,
  SarsaAgentG3 = 67,

  QLearningAgentB1 = 70,
  QLearningAgentB2 = 71,
  QLearningAgentB3 = 72,

  SarsaAgentB1 = 75,
  SarsaAgentB2 = 76,
  SarsaAgentB3 = 77,

  QLearningStats = 80,
  GenerationCoverageBoxStats = 90,
  GenerationCoverageStepperStats = 100,
  GenerationLinesBarStats = 110,
  GenerationEfficencyTable = 120,
  GenerationCoverageHeatMap = 130,
  GenerationTimesBarStats = 135,
  GenerationCompressionBarStats = 136,
  GenerationExecTimesBarStats = 137,
  GenerationEfficienctRadarPlotStats = 138,

  AggregateData = 140,
  VerifyData = 145,
  ComputeCompression = 146,
  RemoveGroupData = 150,
  CleanupData = 160,
  ShowResults = 170,

  ModifyData = 180,
};

enum class MethodsGroup {
  QLEG = 1,
  QLB,
  SLEG,
  SLB,
  MCTS,
  QLEG2_VS_QLB2,
  RAND,
  GREEDY_R,
  DSL,
  SELECTED,
  TARGET,
  ALL,
};

struct ObjectiveValue final {
  double coverage = 0.0;
  std::vector<std::uint8_t> coveredTracks;
};

struct FineObjectiveValue final {
  double coverage = 0.0;
  std::vector<std::vector<std::uint8_t>> fineCoveredTracks;
};

struct Cmd final {
  Cmd(int argc, char* argv[]);

  PipelineType pipelineType = PipelineType::HS0;
  std::string workingDir;
  std::string sourcesDir;
  std::string objectDir;
  std::string binPath;
  std::string covDir;
  std::string resultsDir;
  std::string commonResultsDir;
  MethodsGroup group = MethodsGroup::ALL;
};

class Seed final {
 public:
  static Seed& instance() {
    static Seed obj;
    return obj;
  }

  auto& get() { return _gen; }

 private:
  Seed() : _gen(_rd()) {}

  std::random_device _rd;
  mutable std::mt19937 _gen;
};

bool isDebuggerAttached();

std::string loadFile(const std::string& path);

template <typename Func>
bool retry(Func&& func, int maxAttempts = 5) {
    for (int i = 0; i < maxAttempts; ++i) {
        if (func())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return false;
}

}  // namespace cider
