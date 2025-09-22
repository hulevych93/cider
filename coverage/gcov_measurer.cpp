// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "gcov_measurer.h"

#include <assert.h>

#include <tlog.h>
#include <thread>

#include "recorder/details/generator.h"
#include "scripting/runner.h"

namespace cider {
namespace gcov_coverage {

namespace {
template <typename Func>
bool rerty(Func&& func) {
  int count = 0;
  auto success = false;
  while (!success && count < 5) {
    success = func();
    count++;
  }
  return success;
}
}  // namespace

CoverageMeasurment::CoverageMeasurment(const Cmd& cmd, const char* module)
    : _cmd(cmd), _module(module) {}

ReportOpt CoverageMeasurment::getReport(
    const std::vector<cider::recorder::Action>& actions) {
  const auto script = getScript(actions);

  rerty([&]() -> bool { return cleanCoverage(_cmd.covDir); });

  const auto result = scripting::runScript(
      _cmd.binPath, _cmd.workingDir, script, [](const char*, std::size_t) {});

  // if (result) {
  std::string jsonReport;

  rerty([&]() -> bool {
    return runCoverage(_cmd.baseDir, _cmd.objectDir,
                       [&](const char* data, std::size_t size) {
                         jsonReport += std::string{data, size};
                       });
  });

  const auto rootReport = parseJsonCovReport(jsonReport, false);

  if (rootReport.has_value() && m_logger) {
    m_logger->log(_index, rootReport.value());
  }

  return rootReport;
  // }

  return std::nullopt;
}

std::string CoverageMeasurment::getScript(
    const std::vector<cider::recorder::Action>& actions) const {
  auto generator = cider::recorder::makeLuaGenerator(_module);
  ++_index;
  return cider::recorder::generateScript(generator, actions, 999999U);
}

StepperCoverageMeasurment::StepperCoverageMeasurment(const Cmd& cmd,
                                                     const char* module)
    : CoverageMeasurment(cmd, module) {}

void StepperCoverageMeasurment::measure(
    const std::vector<cider::recorder::Action>& actions,
    const size_t stepSize,
    const std::optional<size_t> covReachLen) {
  tlog_info << "actions size: " << actions.size() << std::endl;

  if (m_logger) {
    m_logger->log(0U, {});
  }

  _index = 0;

  for (; _index < actions.size();) {
    (*this)(actions);
    const size_t nextIndex = _index + stepSize;

    if (covReachLen.has_value()) {
      const auto richLenIntex = covReachLen.value() - 1;
      if (richLenIntex > _index && richLenIntex < nextIndex &&
          richLenIntex < actions.size()) {
        _index = richLenIntex;
        tlog_info << "[CovReach] forced log at i=" << _index << "\n";
        (*this)(actions);
      }
    }

    _index = nextIndex;
  }

  if (_index - stepSize < actions.size() - 1) {
    _index = actions.size() - 1;
    tlog_info << "[CovReach] final log at i=" << _index << "\n";
    (*this)(actions);
  }
}

std::string StepperCoverageMeasurment::getScript(
    const std::vector<cider::recorder::Action>& actions) const {
  auto generator = cider::recorder::makeLuaGenerator(_module);
  return cider::recorder::generateScript(generator, actions, _index);
}

}  // namespace gcov_coverage
}  // namespace cider
