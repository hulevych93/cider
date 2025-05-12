// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "logger.h"

#include <assert.h>

#include <iostream>

#include "recorder/details/generator.h"

namespace cider {
namespace cfg_coverage {

FileLogger::FileLogger(const std::string& logDir,
                       const std::string& logFileName) {
  std::filesystem::path outPath(logDir);
  std::filesystem::create_directories(outPath);
  outPath /= logFileName;
  std::cout << "Out file: " << outPath << std::endl;
  _report.open(outPath, std::ios::out | std::ios::app);
}

void FileLogger::log(size_t index, const Coverage& coverage) const {
  printTableEntry(_report, index, coverage);
}

}  // namespace cfg_coverage

namespace gcov_coverage {

FileLogger::FileLogger(const std::string& logDir,
                       const std::string& logFileName) {
  std::filesystem::path outPath(logDir);
  std::filesystem::create_directories(outPath);
  outPath /= logFileName;
  std::cout << "Out file: " << outPath << std::endl;
  _report.open(outPath, std::ios::out | std::ios::app);
}

void FileLogger::log(size_t index, const RootReport& coverage) const {
  printTableEntry(_report, index, coverage.report);
}

}  // namespace gcov_coverage

}  // namespace cider
