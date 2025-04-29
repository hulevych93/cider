// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/cfg_coverage.h"

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

struct CoverageMeasurment final {
  CoverageMeasurment(const Cmd& cmd, const char* module);

  void setLogger(std::unique_ptr<ICoverageLogger> logger) {
    m_logger = std::move(logger);
  }

  CfgCoverageOpt operator()(
      const std::vector<cider::recorder::Action>& actions);

  std::string getScript(
      const std::vector<cider::recorder::Action>& actions) const;

 private:
  const Cmd& _cmd;
  std::unique_ptr<ICoverageLogger> m_logger;

 protected:
  mutable size_t _index = 1U;
  const char* _module;
};

}  // namespace cfg_coverage
}  // namespace cider
