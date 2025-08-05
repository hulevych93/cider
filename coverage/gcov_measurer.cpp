// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "gcov_measurer.h"

#include <assert.h>

#include <iostream>
#include <thread>

#include "recorder/details/generator.h"
#include "scripting/runner.h"

namespace cider {
namespace gcov_coverage {

CoverageMeasurment::CoverageMeasurment(const Cmd& cmd, const char* module)
    : _cmd(cmd), _module(module) {}

ReportOpt CoverageMeasurment::getReport(
    const std::vector<cider::recorder::Action>& actions,
    unsigned long& executionTimeMs) {
  const auto script = getScript(actions);

  assert(cleanCoverage(_cmd.covDir));

  const auto start = std::chrono::steady_clock::now();
  const auto result = scripting::runScript(
      _cmd.binPath, _cmd.workingDir, script, [](const char*, std::size_t) {});
  auto end = std::chrono::steady_clock::now();
  if (result) {
    std::string jsonReport;
    assert(runCoverage(_cmd.baseDir, _cmd.objectDir,
                       [&](const char* data, std::size_t size) {
                         jsonReport += std::string{data, size};
                       }));

    const auto rootReport = parseJsonCovReport(jsonReport, false);
    assert(rootReport.has_value());

    if (m_logger) {
      m_logger->log(_index, rootReport.value());
    }

    executionTimeMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    return rootReport.value();
  }

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
    const std::vector<cider::recorder::Action>& actions) {
  std::cout << "actions size: " << actions.size() << std::endl;

  if (m_logger) {
    m_logger->log(0U, {});
  }

  m_stepSize = 20;

  for (; _index < actions.size();) {
    (*this)(actions);

    assert(m_stepSize > 0);
    _index += m_stepSize;
  }
}

std::string StepperCoverageMeasurment::getScript(
    const std::vector<cider::recorder::Action>& actions) const {
  auto generator = cider::recorder::makeLuaGenerator(_module);
  return cider::recorder::generateScript(generator, actions, _index);
}

}  // namespace gcov_coverage
}  // namespace cider
