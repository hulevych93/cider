// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace cider {

enum class PipelineType {
  HarmonySearch = 0,
  CackooSearch = 1,
  QLearningAgent = 2,
  QLearningAgentPlusHarmonySearch = 3,
  QLearningAgentPlusCackooSearch = 4
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
};

std::string loadFile(const std::string& path);

}  // namespace cider
