// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace cider {
namespace paths {

std::string getQLearningAgentPath(const std::string& resultsDir);
std::string getSarsaAgentPath(const std::string& resultsDir);

std::string getResultsPath(const std::string& resultsDir);

std::string getBriefResultsPath(const std::string& resultsDir);

std::string getSessionsResultsPath(const std::string& resultsDir);

std::string getQLearningDir(const std::string& resultsDir,
                            const std::string& metadata,
                            const std::string& prefix);

}  // namespace paths
}  // namespace cider
