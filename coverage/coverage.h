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

  CackooSearch = 10,

  GRAND = 20,

  QLearningAgentLearning = 50,

  QLearningAgentG1 = 60,
  QLearningAgentG2 = 61,
  QLearningAgentG3 = 62,

  QLearningAgentB1 = 70,
  QLearningAgentB2 = 71,
  QLearningAgentB3 = 72,

  GenerationCoverageBoxStats = 90,
  GenerationCoverageStepperStats = 100,
  GenerationLinesBarStats = 110,
};

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
