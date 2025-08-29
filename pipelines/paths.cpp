// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "paths.h"

#include <filesystem>

namespace cider {
namespace paths {

std::string getQLearningAgentPath(const std::string& resultsDir) {
  std::filesystem::path outPath(resultsDir);
  outPath /= "qtable_agent.img";
  return outPath.string();
}

std::string getSarsaAgentPath(const std::string& resultsDir) {
  std::filesystem::path outPath(resultsDir);
  outPath /= "sarsa_qtable_agent.img";
  return outPath.string();
}

std::string getBriefResultsPath(const std::string& resultsDir) {
  std::filesystem::path outPath(resultsDir);
  outPath /= "brief_results.img";
  return outPath.string();
}

std::string getSessionsResultsPath(const std::string& resultsDir) {
  std::filesystem::path outPath(resultsDir);
  outPath /= "sessions_results.img";
  return outPath.string();
}

}  // namespace paths
}  // namespace cider
