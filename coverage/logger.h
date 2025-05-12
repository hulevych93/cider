// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "coverage/cfg_coverage.h"
#include "coverage/gcov_coverage.h"

#include <fstream>

namespace cider {
namespace cfg_coverage {

using CfgCoverageOpt = std::optional<cfg_coverage::Coverage>;

class ICoverageLogger {
 public:
  virtual ~ICoverageLogger() = default;
  virtual void log(size_t index, const Coverage& coverage) const = 0;
};

class FileLogger : public ICoverageLogger {
 public:
  FileLogger(const std::string& logDir, const std::string& logFileName);

  void log(size_t index, const Coverage& coverage) const override;

 private:
  mutable std::ofstream _report;
};

}  // namespace cfg_coverage

namespace gcov_coverage {

using ReportOpt = std::optional<RootReport>;

class ICoverageLogger {
 public:
  virtual ~ICoverageLogger() = default;
  virtual void log(size_t index, const RootReport& coverage) const = 0;
};

class FileLogger : public ICoverageLogger {
 public:
  FileLogger(const std::string& logDir, const std::string& logFileName);

  void log(size_t index, const RootReport& coverage) const override;

  void other() { _report << "Optimized script log" << std::endl; }

 private:
  mutable std::ofstream _report;
};

}  // namespace gcov_coverage

}  // namespace cider
