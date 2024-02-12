// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include <iostream>

#include "coverage/coverage.h"

#include <process.hpp>

#include <assert.h>

namespace tpl = TinyProcessLib;

bool cleanCoverage() {
  tpl::Process process(
      "find . -name \"*.gcda\" -delete", "",
      [](const char* data, std::size_t) { std::cout << data; },
      [](const char* data, std::size_t) { std::cerr << data; });
  return process.get_exit_status() == 0;
}

bool runScript(const std::string& binDir,
               const std::string& workingDir,
               const std::string& scriptPath) {
  tpl::Process process(
      binDir + "/hjson_player " + scriptPath, workingDir,
      [](const char* data, std::size_t) { std::cout << data; },
      [](const char* data, std::size_t) { std::cerr << data; });
  return process.get_exit_status() == 0;
}

bool runCoverage(const std::string& base,
                 const std::string& objectDir,
                 std::function<void(const char*, std::size_t)> callback) {
  tpl::Process process(
      "gcovr --json-summary --json-summary-pretty -r" + base +
          " --object-directory=" + objectDir,
      "", callback, [](const char* data, std::size_t) { std::cerr << data; });
  return process.get_exit_status() == 0;
}

void printTableEntry(std::ostream& ss,
                     const std::string& name,
                     const cider::coverage::CoverageReport& report) {
  ss << name << "\t" << report.lineCov.percent << "\t"
     << report.branchCov.percent << "\t" << report.funcCov.percent << std::endl;
}

int main(int argc, char* argv[]) {
  try {
    assert(argc == 6);
    std::cout << "working dir: " << argv[1] << std::endl;
    std::cout << "base dir: " << argv[2] << std::endl;
    std::cout << "object dir: " << argv[3] << std::endl;
    std::cout << "script path: " << argv[4] << std::endl;
    std::cout << "bin path: " << argv[5] << std::endl;

    assert(cleanCoverage());

    assert(runScript(argv[5], argv[1], argv[4]));

    std::string jsonReport;
    assert(
        runCoverage(argv[2], argv[3], [&](const char* data, std::size_t size) {
          jsonReport += std::string{data, size};
        }));

    const auto rootReport =
        cider::coverage::parseJsonCovReport(jsonReport, true);
    assert(rootReport.has_value());

    printTableEntry(std::cout, "root", rootReport->report);

    for (const auto& file : rootReport->files) {
      printTableEntry(std::cout, file.name, file.report);
    }

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}
