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

std::string loadFile(const std::string& path) {
  if (!std::filesystem::exists(path)) {
    std::cout << path << "doesn't exist" << std::endl;
  }
  std::ifstream scr1(path, std::ios::binary);
  scr1.seekg(0, std::ios::end);
  size_t size = scr1.tellg();
  std::string script(size, ' ');
  scr1.seekg(0);
  scr1.read(&script[0], size);
  return script;
}

bool cleanCoverage(const std::string& workingDir) {
  tpl::Process process(
      std::string{"find "} + workingDir + " -name \"*.gcda\" -delete", "",
      [](const char* data, std::size_t) { std::cout << data; },
      [](const char* data, std::size_t) { std::cout << data; });
  return process.get_exit_status() == 0;
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
