// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "cfg_coverage.h"

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
  if (result < percentageThreshold)
    result = 0.0f;
  return result * 100.0f;
}

Coverage& Coverage::alignTo(const Coverage& startingPoint) {
  assert(total == startingPoint.total);
  assert(covered >= startingPoint.covered);
  covered = covered - startingPoint.covered;
  return *this;
}

bool serialize(const Coverage& obj, serialization::Serializer& serializer) {
  serializer << obj.covered;
  serializer << obj.total;
  serializer << obj.status;
  serializer << obj.coveredTracks;
  return true;
}

bool deserialize(Coverage& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.covered;
  deserializer >> obj.total;
  deserializer >> obj.status;
  deserializer >> obj.coveredTracks;
  return true;
}

void Coverage::dump() const {
  std::cout << covered << ":" << total << std::endl;
}

void dumpCoverageToCout(bool status, const Coverage& startPoint) {
  auto coverage = getCoverage();
  coverage.alignTo(startPoint).status = status;
  const auto covJson = serializeCovReport(coverage);
  std::cout << MarkerStart << covJson << MarkerEnd << coverage.getPercentage();
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

void printTableEntry(std::ostream& ss,
                     const size_t index,
                     const cider::cfg_coverage::Coverage& report) {
  ss << index << "\t" << report.getPercentage() << std::endl;
}

}  // namespace cfg_coverage
}  // namespace cider
