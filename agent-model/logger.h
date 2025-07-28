// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <fstream>

namespace cider {
namespace agent_model {

class IResultsLogger {
 public:
  virtual ~IResultsLogger() = default;

  virtual void logReward(size_t episode, double totalReward) = 0;
  virtual void logLoss(size_t episode, double averageLoss) = 0;

  virtual void save() = 0;
};

class ICovLogger {
public:
    virtual ~ICovLogger() = default;

    virtual void set(double maxCov) = 0;

    virtual void log(size_t episode, double cov) = 0;

    virtual void save() = 0;
};

class FileLogger : public IResultsLogger {
 public:
  FileLogger(const std::string& logDir, const std::string& logFileName);

  void logReward(size_t episode, double totalReward) override;
  void logLoss(size_t episode, double averageLoss) override;

  void save() override {}

 private:
  mutable std::ofstream _report;
};

}  // namespace agent_model
}  // namespace cider
