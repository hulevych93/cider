// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "gcov_coverage.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <process.hpp>

#include <filesystem>
#include <fstream>

namespace tpl = TinyProcessLib;

namespace cider {
namespace gcov_coverage {

bool serialize(const Coverage& obj, serialization::Serializer& serializer) {
  serializer << obj.covered;
  serializer << obj.total;
  serializer << obj.percent;
  return true;
}

bool deserialize(Coverage& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.covered;
  deserializer >> obj.total;
  deserializer >> obj.percent;
  return true;
}

bool serialize(const CoverageReport& obj,
               serialization::Serializer& serializer) {
  serializer << obj.lineCov;
  serializer << obj.branchCov;
  serializer << obj.funcCov;
  return true;
}

bool deserialize(CoverageReport& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.lineCov;
  deserializer >> obj.branchCov;
  deserializer >> obj.funcCov;
  return true;
}

CoverageReport parseCoverageReport(const nlohmann::json& value) {
  CoverageReport report;
  report.branchCov.covered = value["branch_covered"];
  report.branchCov.total = value["branch_total"];
  report.branchCov.percent = value["branch_percent"];

  report.funcCov.covered = value["function_covered"];
  report.funcCov.total = value["function_total"];
  report.funcCov.percent = value["function_percent"];

  report.lineCov.covered = value["line_covered"];
  report.lineCov.total = value["line_total"];
  report.lineCov.percent = value["line_percent"];
  return report;
}

std::optional<RootReport> parseJsonCovReport(const std::string& json,
                                             const bool deep) {
  RootReport report;
  try {
    const auto parsedReport = nlohmann::json::parse(json);
    report.root = parsedReport["root"];
    report.report = parseCoverageReport(parsedReport);

    if (deep) {
      for (const auto& file : parsedReport["files"]) {
        FileReport fileReport;
        fileReport.name = file["filename"];
        fileReport.report = parseCoverageReport(file);
        report.files.emplace_back(fileReport);
      }
    }
  } catch (...) {
    return std::nullopt;
  }
  return report;
}

bool cleanCoverage(const std::string& workingDir) {
  namespace fs = std::filesystem;
  try {
    for (auto& p : fs::recursive_directory_iterator(workingDir)) {
      if (p.path().extension() == ".gcda") {
        fs::remove(p.path());
      }
    }
    return true;
  } catch (const std::exception& e) {
    std::cerr << "[cleanCoverage] Error: " << e.what() << "\n";
    return false;
  }
}

bool runCoverage(const std::string& base,
                 const std::string& objectDir,
                 std::function<void(const char*, std::size_t)> callback) {
  tpl::Process process(
      "gcovr --gcov-delete --json-summary --json-summary-pretty -r" + base +
          " --object-directory=" + objectDir,
      "", callback, nullptr);
  return process.get_exit_status() == 0;
}

void printTableEntry(std::ostream& ss,
                     const size_t index,
                     const CoverageReport& report) {
  ss << index << "\t" << report.lineCov.percent << "\t"
     << report.branchCov.percent << "\t" << report.funcCov.percent << std::endl;
}

}  // namespace gcov_coverage
}  // namespace cider
