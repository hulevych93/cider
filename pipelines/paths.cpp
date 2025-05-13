// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "paths.h"

#include <filesystem>

namespace cider {
namespace paths {

std::string getQTableAgentPath(const std::string& resultsDir) {
  std::filesystem::path outPath(resultsDir);
  outPath /= "qtable_agent.img";
  return outPath.string();
}

}  // namespace paths
}  // namespace cider
