// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "gcov_measurer.h"

#include <assert.h>

#include <iostream>
#include <thread>

#include "recorder/details/generator.h"

namespace cider {
namespace gcov_coverage {

CoverageMeasurment::CoverageMeasurment(const Cmd& cmd,
                                       const char* logName,
                                       const char* module)
    : _cmd(cmd), _module(module) {

    std::filesystem::path outPath(cmd.resultsDir);
    std::filesystem::create_directories(outPath);
    outPath /= logName;
    std::cout << "Out file: " << outPath << std::endl;
    _report.open(outPath, std::ios::out | std::ios::trunc);
}

ReportOpt CoverageMeasurment::operator()(
    const std::vector<cider::recorder::Action>& actions) {
  const auto script = getScript(actions);

  assert(cider::coverage::cleanCoverage(_cmd.covDir));

  const auto result =
      cider::coverage::runScript(_cmd.binPath, _cmd.workingDir, script);
  if (result) {
    std::string jsonReport;
    assert(cider::coverage::runCoverage(
        _cmd.baseDir, _cmd.objectDir, [&](const char* data, std::size_t size) {
          jsonReport += std::string{data, size};
        }));

    const auto rootReport =
        cider::coverage::parseJsonCovReport(jsonReport, false);
    assert(rootReport.has_value());

    cider::coverage::printTableEntry(_report, _index, rootReport->report);

    for (const auto& fileReport : rootReport->files) {
      auto filePath = std::filesystem::path{fileReport.name};
      filePath.replace_extension("txt");
      std::ofstream fileStream(filePath, std::ios::app);
      cider::coverage::printTableEntry(fileStream, _index, fileReport.report);
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
                                                     const char* logName,
                                                     const char* module)
    : CoverageMeasurment(cmd, logName, module) {
}

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
