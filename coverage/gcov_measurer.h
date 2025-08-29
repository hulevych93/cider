// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/gcov_coverage.h"
#include "coverage/logger.h"

#include <fstream>

namespace cider {
namespace gcov_coverage {

using ReportOpt = std::optional<RootReport>;

struct CoverageMeasurment {
  CoverageMeasurment(const Cmd& cmd, const char* module);

  void setLogger(const std::shared_ptr<ICoverageLogger>& logger) {
    m_logger = logger;
  }

  double operator()(const std::vector<cider::recorder::Action>& actions) {
    const auto rootReport = getReport(actions);
    if (rootReport.has_value()) {
      return rootReport->report.branchCov.percent;
    }
    return 0.0f;
  }

  auto getObjValueFunc() {
    return [this](const std::vector<cider::recorder::Action>& actions)
               -> ObjectiveValue {
      const auto rootReport = getReport(actions);

      ObjectiveValue value;
      if (rootReport.has_value()) {
        value.coverage = rootReport->report.branchCov.percent;
      }
      return value;
    };
  }

  ReportOpt getReport(const std::vector<cider::recorder::Action>& actions);

  virtual std::string getScript(
      const std::vector<cider::recorder::Action>& actions) const;

 protected:
  const Cmd& _cmd;
  mutable size_t _index = 0U;
  const char* _module;
  std::shared_ptr<ICoverageLogger> m_logger;
};

struct StepperCoverageMeasurment final : CoverageMeasurment {
  StepperCoverageMeasurment(const Cmd& cmd, const char* module);

  void measure(const std::vector<cider::recorder::Action>& actions);

  std::string getScript(
      const std::vector<cider::recorder::Action>& actions) const override;

 private:
  unsigned int m_stepSize = 0;
};

}  // namespace gcov_coverage
}  // namespace cider
