// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "logger.h"

#include <filesystem>
#include <iostream>

namespace cider {
namespace agent_model {

FileLogger::FileLogger(const std::string& logDir,
                       const std::string& logFileName) {
  std::filesystem::path outPath(logDir);
  std::filesystem::create_directories(outPath);
  outPath /= logFileName;
  std::cout << "Out file: " << outPath << std::endl;
  _report.open(outPath, std::ios::out | std::ios::app);
}

void FileLogger::logReward(size_t episode, double totalReward) {
  _report << "EPS: " << episode << "\t"
          << "RWRD: " << totalReward << std::endl;
}

void FileLogger::logLoss(size_t episode, double averageLoss) {
  _report << "EPS: " << episode << "\t"
          << "LOSS: " << averageLoss << std::endl;
}

}  // namespace agent_model
}  // namespace cider
