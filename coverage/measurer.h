// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"

#include <fstream>

namespace cider {
namespace coverage {

struct Cmd final {
  Cmd(int argc, char* argv[]);

  std::string workingDir;
  std::string baseDir;
  std::string objectDir;
  std::string binPath;
  std::string covDir;
};

using ReportOpt = std::optional<coverage::RootReport>;

struct CoverageMeasurment {
  explicit CoverageMeasurment(const Cmd& cmd,
                              const char* logName,
                              const char* module);

  ReportOpt operator()(const std::vector<cider::recorder::Action>& actions);

  virtual std::string getScript(
      const std::vector<cider::recorder::Action>& actions) const;

 private:
  const Cmd& _cmd;
  mutable std::ofstream _report;

 protected:
  mutable size_t _index = 1U;
  const char* _module;
};

struct StepperCoverageMeasurment final : CoverageMeasurment {
  explicit StepperCoverageMeasurment(const Cmd& cmd, const char* module);

  void measure(const std::vector<cider::recorder::Action>& actions);

  std::string getScript(
      const std::vector<cider::recorder::Action>& actions) const override;
};

}  // namespace coverage
}  // namespace cider
