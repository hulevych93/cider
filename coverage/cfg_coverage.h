// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "coverage/coverage.h"

namespace cider {
namespace cfg_coverage {

struct Coverage final {
  constexpr static const double percentageThreshold = 0.005;
  std::uint32_t covered = 0U;
  std::uint32_t total = 0U;
  bool status = true;

  double getPercentage() const {
    if (total == 0)
      return 0.0f;
    auto result = double(covered) / double(total);
    if (result < percentageThreshold)
      result = 0.0f;
    return result;
  }

  Coverage& operator=(const Coverage& rhs) {
    if (this != &rhs) {
      covered = rhs.covered;
      total = rhs.total;
    }
    return *this;
  }

  void dump() const;

  Coverage& alignTo(const Coverage& startingPoint);
};

void dumpCoverageToCout(bool status, const Coverage& startPoint);

Coverage getCoverage();

std::string readCoverageJsonFromStream(const std::string& input);

std::optional<Coverage> parseJsonCovReport(const std::string& json);

std::string setializeJsonCovReport(const Coverage& report);

bool runScript(const std::string& binary,
               const std::string& workingDir,
               const std::string& script,
               std::function<void(const char*, std::size_t)> callback);

void printTableEntry(std::ostream& ss,
                     const size_t index,
                     const Coverage& report);

inline bool operator==(const Coverage& lhs, const Coverage& rhs) {
  return lhs.covered == rhs.covered && lhs.total == rhs.total;
}

}  // namespace cfg_coverage
}  // namespace cider
