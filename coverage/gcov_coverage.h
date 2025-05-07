// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "coverage/coverage.h"

namespace cider {
namespace gcov_coverage {

struct Coverage final {
  std::uint32_t covered = 0U;
  std::uint32_t total = 0U;
  double percent = 0.0;

  Coverage& operator=(const Coverage& rhs) {
    if (this != &rhs) {
      covered = rhs.covered;
      total = rhs.total;
      percent = rhs.percent;
    }
    return *this;
  }
};

struct CoverageReport final {
  Coverage lineCov;
  Coverage branchCov;
  Coverage funcCov;

  CoverageReport& operator=(const CoverageReport& rhs) {
    if (this != &rhs) {
      lineCov = rhs.lineCov;
      branchCov = rhs.branchCov;
      funcCov = rhs.funcCov;
    }
    return *this;
  }
};

struct FileReport final {
  std::string name;
  CoverageReport report;

  FileReport& operator=(const FileReport& rhs) {
    if (this != &rhs) {
      name = rhs.name;
      report = rhs.report;
    }
    return *this;
  }
};

struct RootReport final {
  std::string root;
  CoverageReport report;
  std::vector<FileReport> files;

  RootReport& operator=(const RootReport& rhs) {
    if (this != &rhs) {
      root = rhs.root;
      report = rhs.report;
      files = rhs.files;
    }
    return *this;
  }
};

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

inline bool operator>(const CoverageReport& left, const CoverageReport& right) {
  return left.lineCov.percent > right.lineCov.percent;
}

inline bool operator>(const RootReport& left, const RootReport& right) {
  return left.report > right.report;
}

inline bool operator<(const CoverageReport& left, const CoverageReport& right) {
  return left.lineCov.covered < right.lineCov.covered;
}

inline bool operator<(const RootReport& left, const RootReport& right) {
  return left.report < right.report;
}

inline bool operator==(const Coverage& lhs, const Coverage& rhs) {
  return lhs.covered == rhs.covered && lhs.total == rhs.total &&
         lhs.percent == rhs.percent;
}

inline bool operator==(const CoverageReport& lhs, const CoverageReport& rhs) {
  return lhs.lineCov == rhs.lineCov && lhs.branchCov == rhs.branchCov &&
         lhs.funcCov == rhs.funcCov;
}

inline bool operator==(const FileReport& lhs, const FileReport& rhs) {
  return lhs.name == rhs.name && lhs.report == rhs.report;
}

inline bool operator==(const RootReport& lhs, const RootReport& rhs) {
  return lhs.root == rhs.root && lhs.report == rhs.report &&
         lhs.files == rhs.files;
}

}  // namespace gcov_coverage
}  // namespace cider
