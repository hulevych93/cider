// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "gcov_measurer_llvm.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

#include "recorder/details/generator.h"
#include "scripting/runner.h"

namespace cider {
namespace llvm_gc_coverage {

CoverageMeasurment::CoverageMeasurment(const Cmd& cmd, const char* module)
    : _cmd(cmd), _module(module) {
  retry([&]() { return cleanCoverage(_cmd.covDir); });
}

ReportOpt CoverageMeasurment::getReport(
    const std::vector<cider::recorder::Action>& actions) {
  const auto script = getScript(actions);

  const auto now = std::chrono::system_clock::now();

  const auto result = llvm_coverage::runScript(
      _cmd.binPath, _cmd.workingDir, script, [](const char*, std::size_t) {});

  if (result || isDebuggerAttached()) {
    GcovBranchParser parser;

    const auto result = runCoverage(
        _cmd.objectDir, _cmd.sourcesDir,
        [&](const char* data, std::size_t sz) { parser.feed(data, sz); });

    if (result || isDebuggerAttached()) {
      auto rootReport = parser.finish();
      if (m_logger) {
        m_logger->log(_index, rootReport);
      }

      const auto end = std::chrono::system_clock::now();
      const auto meassureTimeMcs =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - now)
              .count();

      if (false) {
        std::cout << "Cov time: " << meassureTimeMcs << " mills" << std::endl;
      }

      return rootReport;
    }
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
    const std::vector<cider::recorder::Action>& actions,
    const size_t stepSize,
    const std::optional<size_t> covReachLen) {
  if (m_logger)
    m_logger->log(0U, {});

  _index = 0;
  for (; _index < actions.size();) {
    (*this)(actions);
    const size_t nextIndex = _index + stepSize;

    if (covReachLen) {
      size_t richLenIndex = *covReachLen - 1;
      if (richLenIndex > _index && richLenIndex < nextIndex &&
          richLenIndex < actions.size()) {
        _index = richLenIndex;
        (*this)(actions);
      }
    }
    _index = nextIndex;
  }

  if (_index - stepSize < actions.size() - 1) {
    _index = actions.size() - 1;
    (*this)(actions);
  }
}

std::string StepperCoverageMeasurment::getScript(
    const std::vector<cider::recorder::Action>& actions) const {
  auto generator = cider::recorder::makeLuaGenerator(_module);
  return cider::recorder::generateScript(generator, actions, _index);
}

}  // namespace llvm_gc_coverage
}  // namespace cider
