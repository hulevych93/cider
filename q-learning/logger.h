// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <fstream>

namespace cider {
namespace qleaning {

class IResultsLogger {
 public:
  virtual ~IResultsLogger() = default;

  virtual void logReward(size_t episode, double totalReward) const = 0;
  virtual void logLoss(size_t episode, double averageLoss) const = 0;
};

class FileLogger : public IResultsLogger {
 public:
  FileLogger(const std::string& logDir, const std::string& logFileName);

  void logReward(size_t episode, double totalReward) const override;
  void logLoss(size_t episode, double averageLoss) const override;

 private:
  mutable std::ofstream _report;
};

}  // namespace qleaning
}  // namespace cider
