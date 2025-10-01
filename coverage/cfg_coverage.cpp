// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "cfg_coverage.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <assert.h>

constexpr size_t MAX_GUARDS = 65536;
std::uint8_t coverage_map[MAX_GUARDS] = {0};
std::uint32_t max_guard_id = 0;

extern "C" {

// Called once at startup to initialize guard values
extern "C" void __sanitizer_cov_trace_pc_guard_init(uint32_t* start,
                                                    uint32_t* end) {
  static uint32_t N;
  if (start == end || *start)
    return;
  for (uint32_t* x = start; x < end; ++x) {
    *x = ++N;
    if (N > max_guard_id)
      max_guard_id = N;  // track largest assigned ID
  }
}

// Called at each instrumented point during execution
void __sanitizer_cov_trace_pc_guard(std::uint32_t* guard) {
  if (!*guard)
    return;
  uint32_t id = *guard;
  if (id < MAX_GUARDS) {
    coverage_map[id] = 1;  // Mark as covered
  }
}

}  // extern "C"

constexpr const char* MarkerStart = "CFG_COV_START";
constexpr const char* MarkerEnd = "CFG_COV_END";

namespace cider {
namespace cfg_coverage {

Coverage& Coverage::operator=(const Coverage& rhs) {
  if (this != &rhs) {
    covered = rhs.covered;
    total = rhs.total;
    status = rhs.status;
    coveredTracks = rhs.coveredTracks;
  }
  return *this;
}

double Coverage::getPercentage() const {
  if (total == 0)
    return 0.0f;
  auto result = double(covered) / double(total);
  return result * 100.0f;
}

bool serialize(const Coverage& obj, serialization::Serializer& serializer) {
  serializer << obj.meassureTimeMcs;
  serializer << obj.covered;
  serializer << obj.total;
  serializer << obj.status;
  serializer << obj.coveredTracks;
  return true;
}

bool deserialize(Coverage& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.meassureTimeMcs;
  deserializer >> obj.covered;
  deserializer >> obj.total;
  deserializer >> obj.status;
  deserializer >> obj.coveredTracks;
  return true;
}

bool serialize(const CoverageExtended& obj, serialization::Serializer& serializer) {
    serialize(static_cast<const Coverage&>(obj), serializer);
    serializer << obj.hasUnique;
    return true;
}

bool deserialize(CoverageExtended& obj,
                 const serialization::Deserializer& deserializer) {
    deserialize(static_cast<Coverage&>(obj), deserializer);
    deserializer >> obj.hasUnique;
    return true;
}

void zeroCfgCounters(int* blockCount) {
  int j = 0;
  for (int i = 0; i < max_guard_id; ++i) {
    if (coverage_map[i] == 1) {
      ++j;
      coverage_map[i] = 0;  // not relative block
    }
  }
  if (blockCount) {
    *blockCount = max_guard_id;
  }
  std::cout << "Not relative block count " << j << std::endl;
}

void dumpCoverageToCout(
    bool status,
    const std::chrono::steady_clock::time_point& startTime) {
  auto coverage = getCoverage();

  const auto end = std::chrono::steady_clock::now();
  coverage.meassureTimeMcs =
      std::chrono::duration_cast<std::chrono::microseconds>(end - startTime)
          .count();

  coverage.status = status;
  const auto covJson = serializeCovReport(coverage);
  std::cout << MarkerStart << covJson << MarkerEnd << coverage.getPercentage();
}

void dumpCoverageToCout(const CoveragePerAction& coverage) {
  const auto covJson = serializeCovReport(coverage);
  std::cout << MarkerStart << covJson << MarkerEnd;
}

Coverage getCoverage() {
  Coverage coverage;
  coverage.total = max_guard_id;

  for (size_t i = 1; i <= max_guard_id; ++i) {
    if (coverage_map[i]) {
      ++coverage.covered;
    }
  }

  coverage.coveredTracks =
      std::vector<std::uint8_t>(coverage_map, coverage_map + max_guard_id);

  return coverage;
}

std::string readCoverageFromStream(const std::string& input) {
  std::string result;

  auto startPos = input.find(MarkerStart);
  if (startPos == std::string::npos)
    return {};  // MarkerStart not found

  startPos += std::strlen(MarkerStart);  // Move after start marker

  auto endPos = input.find(MarkerEnd, startPos);
  if (endPos == std::string::npos)
    return {};  // MarkerEnd not found

  result = input.substr(startPos, endPos - startPos);
  return result;
}

std::optional<Coverage> deserializeCovReport(const std::string& buffer) {
  std::optional<Coverage> report;
  try {
    serialization::Deserializer deserializer(buffer.data(), buffer.size());
    Coverage cov;
    deserializer >> cov;
    report = std::move(cov);
  } catch (...) {
  }

  return report;
}

CoveragePerAction deserializeCovPerActReport(const std::string& buffer) {
  CoveragePerAction report;
  try {
    serialization::Deserializer deserializer(buffer.data(), buffer.size());
    deserializer >> report;
  } catch (...) {
  }

  return report;
}

std::string serializeCovReport(const Coverage& report) {
  try {
    serialization::Serializer serializer;
    serializer << report;
    return std::string(reinterpret_cast<const char*>(serializer.getData()),
                       serializer.getSize());
  } catch (...) {
  }

  return {};
}

std::string serializeCovReport(const CoveragePerAction& report) {
  try {
    serialization::Serializer serializer;
    serializer << report;
    return std::string(reinterpret_cast<const char*>(serializer.getData()),
                       serializer.getSize());
  } catch (...) {
  }

  return {};
}

void printTableEntry(std::ostream& ss,
                     const size_t index,
                     const cider::cfg_coverage::Coverage& report) {
  ss << index << "\t" << report.getPercentage() << std::endl;
}

}  // namespace cfg_coverage
}  // namespace cider
