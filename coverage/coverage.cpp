// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "coverage.h"

#include <nlohmann/json.hpp>

#include <process.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace tpl = TinyProcessLib;

namespace cider {
namespace coverage {

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

bool runScript(const std::string& binary,
               const std::string& workingDir,
               const std::string& script) {
  tpl::Process process(
      binary, workingDir, [](const char* data, std::size_t) {},
      [](const char* data, std::size_t) {}, true);
  process.write(script.data(), script.size());
  process.close_stdin();
  return process.get_exit_status() == 0;
}

bool runCoverage(const std::string& base,
                 const std::string& objectDir,
                 std::function<void(const char*, std::size_t)> callback) {
  tpl::Process process("gcovr --json-summary --json-summary-pretty -r" + base +
                           " --object-directory=" + objectDir,
                       "", callback, nullptr);
  return process.get_exit_status() == 0;
}

void printTableEntry(std::ostream& ss,
                     const size_t index,
                     const cider::coverage::CoverageReport& report) {
  ss << index << "\t" << report.lineCov.percent << "\t"
     << report.branchCov.percent << "\t" << report.funcCov.percent << std::endl;
}

}  // namespace coverage
}  // namespace cider
