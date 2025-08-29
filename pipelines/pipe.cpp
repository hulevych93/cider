// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "metrics.h"

namespace cider {
namespace pipelines {

bool Pipe::pushResult(const std::string& libName,
                      const cider::Cmd& cmd,
                      const std::string& methodName,
                      Result result) {
  int success = 0;

  cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd, libName.c_str()};
  const auto oldGcovReport = gcov_measurer.getReport(result.oldActions);
  const auto newGcovReport = gcov_measurer.getReport(result.newActions);
  if (oldGcovReport.has_value() && newGcovReport.has_value()) {
    result.oldReport = oldGcovReport->report;
    result.newReport = newGcovReport->report;
    success++;
  }

  cider::cfg_coverage::CoverageMeasurment cgf_measurer{cmd, libName.c_str()};
  const auto oldCfgReport = cgf_measurer.getReport(result.oldActions);
  const auto newCfgReport = cgf_measurer.getReport(result.newActions);
  if (oldCfgReport.has_value() && newCfgReport.has_value()) {
    result.oldCfgReport = oldCfgReport.value();
    result.newCgfReport = newCfgReport.value();
    result.oldExecutionTimeMcs = oldCfgReport.value().meassureTimeMcs;
    result.newExecutionTimeMcs = newCfgReport.value().meassureTimeMcs;
    success++;
  }

  auto reached = computeCoverageReachedLength(result.oldActions,
                                              result.newActions, libName, cmd);
  if (reached.has_value()) {
    result.coverageReachedLength = *reached;
    std::cout << "    [OK] " << result.testCaseName
              << " coverage reached at length = " << *reached << "\n";
    success++;
  }

  printResult(result);
  _owner->pushResult(methodName, std::move(result), success == 2U);
  return true;
}

const Input& Pipe::getInput() const {
  return _owner->_input;
}

const Results& Pipe::getResults() const {
  return _owner->getResults();
}

void Pipe::replaceResults(const Results& newResults) {
  _owner->_results = newResults;
  _owner->_resultsChanged = true;
}

void Pipe::clearData(const std::string& methodName) {
  _owner->clearResults(methodName);
}

}  // namespace pipelines
}  // namespace cider
