// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "coverage.h"

#include <nlohmann/json.hpp>

#include <fstream>

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
  std::ifstream scr1(path, std::ios::binary);
  scr1.seekg(0, std::ios::end);
  size_t size = scr1.tellg();
  std::string script(size, ' ');
  scr1.seekg(0);
  scr1.read(&script[0], size);
  return script;
}

}  // namespace coverage
}  // namespace cider
