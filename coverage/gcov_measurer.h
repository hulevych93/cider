// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/gcov_coverage.h"

#include <fstream>

namespace cider {
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

 private:
  mutable std::ofstream _report;
};

struct CoverageMeasurment {
  CoverageMeasurment(const Cmd& cmd, const char* module);

  void setLogger(const std::string& logDir, const std::string& fileName) {
    auto fileLog = std::make_unique<FileLogger>(logDir, fileName);
    m_logger = std::move(fileLog);
  }

  ReportOpt getReport(const std::vector<cider::recorder::Action>& actions);

  double operator()(const std::vector<cider::recorder::Action>& actions) {
    const auto rootReport = getReport(actions);
    if (rootReport.has_value()) {
      return rootReport->report.lineCov.percent;
    }
    return 0.0f;
  }

  virtual std::string getScript(
      const std::vector<cider::recorder::Action>& actions) const;

 private:
  const Cmd& _cmd;
  std::unique_ptr<ICoverageLogger> m_logger;

 protected:
  mutable size_t _index = 0U;
  const char* _module;
};

struct StepperCoverageMeasurment final : CoverageMeasurment {
  StepperCoverageMeasurment(const Cmd& cmd, const char* module);

  void measure(const std::vector<cider::recorder::Action>& actions);

  std::string getScript(
      const std::vector<cider::recorder::Action>& actions) const override;
};

}  // namespace gcov_coverage
}  // namespace cider
