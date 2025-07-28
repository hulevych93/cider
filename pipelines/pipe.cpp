// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

namespace cider {
namespace pipelines {

void Pipe::pushResult(const std::string& libName,
                      const cider::Cmd& cmd,
                      const std::string& methodName,
                      Result result) {
  cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd, libName.c_str()};
  const auto oldGcovReport = gcov_measurer.getReport(result.oldActions);
  const auto newGcovReport = gcov_measurer.getReport(result.newActions);
  if (oldGcovReport.has_value() && newGcovReport.has_value()) {
    result.oldReport = oldGcovReport->report;
    result.newReport = newGcovReport->report;
  }

  cider::cfg_coverage::CoverageMeasurment cgf_measurer{cmd, libName.c_str()};
  const auto oldCfgReport = cgf_measurer.getReport(result.oldActions);
  const auto newCfgReport = cgf_measurer.getReport(result.newActions);
  if (oldCfgReport.has_value() && newCfgReport.has_value()) {
    result.oldCfgReport = oldCfgReport.value();
    result.newCgfReport = newCfgReport.value();
  }

  _owner->getResults()[methodName].emplace_back(std::move(result));
}

const Input& Pipe::getInput() const {
  return _owner->_input;
}

const Results& Pipe::getResults() const {
  return _owner->getResults();
}

Results& Pipe::getMutableResults() {
  return _owner->getResults();
}

void Pipe::clearData(const std::string& methodName) {
  _owner->getResults()[methodName] = {};
}

}  // namespace pipelines
}  // namespace cider
