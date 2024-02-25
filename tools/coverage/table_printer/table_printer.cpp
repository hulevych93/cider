// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include <string.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "coverage/coverage.h"

#include <assert.h>

namespace {
auto getIndexFromFilepath(const std::string& path) {
  const auto lidx =
      std::filesystem::path(path).filename().replace_extension("");
  static const auto patternLength = strlen("hjson_player_coverage_");
  return std::atoi(lidx.string().substr(patternLength).c_str());
}
}  // namespace

int main(int argc, char* argv[]) {
  try {
    assert(argc == 2);
    std::cout << "working dir: " << argv[1] << std::endl;

    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(argv[1])) {
      files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end(), [](const auto& l, const auto& r) {
      return getIndexFromFilepath(l) < getIndexFromFilepath(r);
    });

    std::ofstream rootTable("coverage_table_root.txt");

    auto idx = 1U;
    for (const auto& path : files) {
      const auto jsonReport = cider::coverage::loadFile(path);
      const auto rootReport =
          cider::coverage::parseJsonCovReport(jsonReport, true);
      assert(rootReport.has_value());

      printTableEntry(rootTable, idx, rootReport->report);

      for (const auto& file : rootReport->files) {
        std::ofstream fileReportTable(file.name + "_table_report.txt",
                                      std::ios::app);
        printTableEntry(fileReportTable, idx, file.report);
      }

      ++idx;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}
