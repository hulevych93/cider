// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cider {
namespace coverage {

struct Coverage final {
  std::uint32_t covered = 0U;
  std::uint32_t total = 0U;
  double percent = 0;
};

struct CoverageReport final {
  Coverage lineCov;
  Coverage branchCov;
  Coverage funcCov;
};

struct FileReport final {
  std::string name;
  CoverageReport report;
};

struct RootReport final {
  std::string root;
  CoverageReport report;
  std::vector<FileReport> files;
};

/**
 * @brief parseJsonCovReport
 * @param json to parse
 * @param deep false means only root report
 * @return report
 */
std::optional<RootReport> parseJsonCovReport(const std::string& json,
                                             bool deep = false);

std::string loadFile(const std::string& path);

bool cleanCoverage(const std::string& workingDir);

bool runScript(const std::string& binary,
               const std::string& workingDir,
               const std::string& script);

bool runCoverage(const std::string& base,
                 const std::string& objectDir,
                 std::function<void(const char*, std::size_t)> callback);

void printTableEntry(std::ostream& ss,
                     const size_t index,
                     const CoverageReport& report);

}  // namespace coverage
}  // namespace cider
