// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

namespace cider {
namespace pipelines {

bool Pipe::pushResult(const std::string& libName,
                      const cider::Cmd& cmd,
                      const std::string& methodName,
                      Result result) {
  int success = 0;

  cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd, libName.c_str()};
  unsigned long oldExecutionTimeMs = 0;
  const auto oldGcovReport =
      gcov_measurer.getReport(result.oldActions, oldExecutionTimeMs);
  unsigned long newExecutionTimeMs = 0;
  const auto newGcovReport =
      gcov_measurer.getReport(result.newActions, newExecutionTimeMs);
  if (oldGcovReport.has_value() && newGcovReport.has_value()) {
    result.oldReport = oldGcovReport->report;
    result.newReport = newGcovReport->report;
    result.oldExecutionTimeMs = oldExecutionTimeMs;
    result.newExecutionTimeMs = newExecutionTimeMs;
    success++;
  }

  cider::cfg_coverage::CoverageMeasurment cgf_measurer{cmd, libName.c_str()};
  const auto oldCfgReport = cgf_measurer.getReport(result.oldActions);
  const auto newCfgReport = cgf_measurer.getReport(result.newActions);
  if (oldCfgReport.has_value() && newCfgReport.has_value()) {
    result.oldCfgReport = oldCfgReport.value();
    result.newCgfReport = newCfgReport.value();
    success++;
  }

  if (success == 2U) {
    printResult(result);
    _owner->pushResult(methodName, std::move(result));
    return true;
  }

  return false;
}

const Input& Pipe::getInput() const {
  return _owner->_input;
}

const Results& Pipe::getResults() const {
  return _owner->getResults();
}

Results& Pipe::getMutableResults() {
  return _owner->getMutableResults();
}

void Pipe::clearData(const std::string& methodName) {
  _owner->clearResults(methodName);
}

}  // namespace pipelines
}  // namespace cider
