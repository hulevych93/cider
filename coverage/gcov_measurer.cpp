// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "gcov_measurer.h"

#include <assert.h>

#include <iostream>
#include <thread>

#include "recorder/details/generator.h"

namespace cider {
namespace gcov_coverage {

FileLogger::FileLogger(const std::string& logDir,
                       const std::string& logFileName) {
  std::filesystem::path outPath(logDir);
  std::filesystem::create_directories(outPath);
  outPath /= logFileName;
  std::cout << "Out file: " << outPath << std::endl;
  _report.open(outPath, std::ios::out | std::ios::trunc);
}

void FileLogger::log(size_t index, const RootReport& coverage) const {
  printTableEntry(_report, index, coverage.report);
}

CoverageMeasurment::CoverageMeasurment(const Cmd& cmd, const char* module)
    : _cmd(cmd), _module(module) {}

ReportOpt CoverageMeasurment::operator()(
    const std::vector<cider::recorder::Action>& actions) {
  const auto script = getScript(actions);

  assert(cleanCoverage(_cmd.covDir));

  const auto result = runScript(_cmd.binPath, _cmd.workingDir, script);
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
  for (; _index < actions.size();) {
    (*this)(actions);
  }
}

std::string StepperCoverageMeasurment::getScript(
    const std::vector<cider::recorder::Action>& actions) const {
  auto generator = cider::recorder::makeLuaGenerator(_module);

  auto result = cider::recorder::generateScript(generator, actions, _index);
  _index += 5;
  return result;
}

}  // namespace gcov_coverage
}  // namespace cider
