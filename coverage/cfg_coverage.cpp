// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "cfg_coverage.h"

#include <nlohmann/json.hpp>

#include <process.hpp>

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

namespace tpl = TinyProcessLib;

constexpr const char* MarkerStart = "CFG_COV_START";
constexpr const char* MarkerEnd = "CFG_COV_END";

namespace cider {
namespace cfg_coverage {

Coverage& Coverage::alignTo(const Coverage& startingPoint) {
  assert(total == startingPoint.total);
  assert(covered >= startingPoint.covered);
  covered = covered - startingPoint.covered;
  return *this;
}

void Coverage::dump() const {
  std::cout << covered << ":" << total << std::endl;
}

void dumpCoverageToCout(bool status, const Coverage& startPoint) {
  auto coverage = getCoverage();
  coverage.alignTo(startPoint).status = status;
  const auto covJson = setializeJsonCovReport(coverage);
  std::cout << MarkerStart << covJson << MarkerEnd << coverage.getPercentage();
}

Coverage getCoverage() {
  Coverage coverage;
  coverage.total = max_guard_id;

  for (size_t i = 1; i <= max_guard_id; ++i) {
    if (coverage_map[i])
      ++coverage.covered;
  }

  return coverage;
}

std::string readCoverageJsonFromStream(const std::string& input) {
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

Coverage parseCoverageReport(const nlohmann::json& value) {
  Coverage report;
  report.covered = value["covered"];
  report.total = value["total"];
  report.status = value["status"];
  return report;
}

std::optional<Coverage> parseJsonCovReport(const std::string& json) {
  std::optional<Coverage> report;
  try {
    const auto parsedReport = nlohmann::json::parse(json);
    report = parseCoverageReport(parsedReport);
  } catch (...) {
    return std::nullopt;
  }
  return report;
}

std::string setializeJsonCovReport(const Coverage& report) {
  nlohmann::json value;
  value["covered"] = report.covered;
  value["total"] = report.total;
  value["status"] = report.status;
  return value.dump();
}

bool runScript(const std::string& binary,
               const std::string& workingDir,
               const std::string& script,
               std::function<void(const char*, std::size_t)> callback) {
  tpl::Process process(
      binary, workingDir, callback, [](const char*, std::size_t) {}, true);
  process.write(script.data(), script.size());
  process.close_stdin();

  if (process.get_exit_status() != 0) {
    std::cout << "status: " << process.get_exit_status() << std::endl;
    static bool fl = false;
    if (!fl) {
      std::ofstream os("trace2.txt");
      os << script;
      fl = true;
    }
  }

  return process.get_exit_status() == 0;
}

void printTableEntry(std::ostream& ss,
                     const size_t index,
                     const cider::cfg_coverage::Coverage& report) {
  ss << index << "\t" << report.getPercentage() << std::endl;
}

}  // namespace cfg_coverage
}  // namespace cider
