// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "serialization/serializable.h"

#include "coverage/coverage.h"

namespace cider {
namespace cfg_coverage {

struct Coverage final : serialization::SerializableTag {
  constexpr static const double percentageThreshold = 0.000000000005;

  std::uint32_t meassureTimeMcs = 0U;
  std::uint32_t covered = 0U;
  std::uint32_t total = 0U;
  bool status = true;

  std::vector<std::uint8_t> coveredTracks;

  double getPercentage() const;

  Coverage& operator=(const Coverage& rhs);

  void dump() const;

  Coverage& alignTo(const Coverage& startingPoint);
};

using CoveragePerAction = std::vector<Coverage>;

bool serialize(const Coverage& obj, serialization::Serializer& serializer);

bool deserialize(Coverage& obj,
                 const serialization::Deserializer& deserializer);

void dumpCoverageToCout(bool status,
                        const Coverage& startPoint,
                        const std::chrono::steady_clock::time_point& startTime);

void dumpCoverageToCout(const CoveragePerAction& coverage);

Coverage getCoverage();

std::string readCoverageFromStream(const std::string& input);

std::optional<Coverage> deserializeCovReport(const std::string& buffer);

CoveragePerAction deserializeCovPerActReport(const std::string& buffer);

std::string serializeCovReport(const Coverage& report);

std::string serializeCovReport(const CoveragePerAction& report);

void printTableEntry(std::ostream& ss,
                     const size_t index,
                     const Coverage& report);

inline bool operator==(const Coverage& lhs, const Coverage& rhs) {
  return lhs.covered == rhs.covered && lhs.total == rhs.total;
}

}  // namespace cfg_coverage
}  // namespace cider
