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

struct CoverageMeasurment final {
  explicit CoverageMeasurment(const Cmd& cmd,
                              const char* logName,
                              const char* module);

  CfgCoverageOpt operator()(
      const std::vector<cider::recorder::Action>& actions);

  std::string getScript(
      const std::vector<cider::recorder::Action>& actions) const;

 private:
  const Cmd& _cmd;
  mutable std::ofstream _report;

 protected:
  mutable size_t _index = 1U;
  const char* _module;
};

}  // namespace cfg_coverage
}  // namespace cider
