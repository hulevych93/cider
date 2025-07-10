// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <random>
#include <string>

namespace cider {

enum class PipelineType {
  HS0 = 0,
  HS1 = 1,
  HS2 = 2,
  HS3 = 3,

  CackooSearch = 10,

  GRAND = 20,

  MCTS0 = 30,
  MCTS1 = 31,
  MCTS2 = 32,

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

  GenerationCoverageBoxStats = 90,
  GenerationCoverageStepperStats = 100,
  GenerationLinesBarStats = 110,
  GenerationEfficencyTable = 120
};

enum class MethodsGroup { QLEG, QLB, SLEG, SLB, MCTS };

struct ObjectiveValue final {
  double coverage = 0.0;
  std::vector<std::uint8_t> coveredTracks;
};

struct Cmd final {
  Cmd(int argc, char* argv[]);

  PipelineType pipelineType = PipelineType::HS0;
  std::string workingDir;
  std::string baseDir;
  std::string objectDir;
  std::string binPath;
  std::string covDir;
  std::string resultsDir;
  std::string commonResultsDir;
  MethodsGroup group = MethodsGroup::MCTS;
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

std::string loadFile(const std::string& path);

}  // namespace cider
