// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace cider {

enum class PipelineType {
  HarmonySearch = 0,  // +QLG
  CackooSearch = 1,   // +QLG
  QLearningAgent = 2,
  QLearningAgentGenerationStepper = 5,
  QLearningAgentGenerationGreedyBoxStats = 6,
  QLearningAgentGenerationBolzmanBoxStats = 7,
  QLearningAgentGenerationBolzmanGreedyBoxStats = 8,

  QLearningAgentGenerationGreedyLineBoxStats = 222,
  QLearningAgentGenerationBolzmanLineBoxStats = 653,

  QLearningScenariosAgent = 23,
  DataSetPlot = 123
};

struct ObjectiveValue final {
  double coverage = 0.0;
  std::vector<std::uint8_t> coveredTracks;
};

struct Cmd final {
  Cmd(int argc, char* argv[]);

  PipelineType pipelineType = PipelineType::HarmonySearch;
  std::string workingDir;
  std::string baseDir;
  std::string objectDir;
  std::string binPath;
  std::string covDir;
  std::string resultsDir;
  std::string commonResultsDir;
};

std::string loadFile(const std::string& path);

}  // namespace cider
