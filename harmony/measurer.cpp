// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "measurer.h"

#include <assert.h>

#include <iostream>
#include <thread>

#include "recorder/details/generator.h"

namespace cider {
namespace harmony {

Cmd::Cmd(int argc, char* argv[]) {
  assert(argc == 6);
  workingDir = argv[1];
  baseDir = argv[2];
  objectDir = argv[3];
  binPath = argv[4];
  covDir = argv[5];

  std::cout << "workingDir: " << workingDir << ", baseDir: " << baseDir
            << ", objectDir: " << objectDir << ", binPath: " << binPath
            << ", covDir: " << covDir << std::endl;
}

CoverageMeasurment::CoverageMeasurment(const Cmd& cmd,
                                       const char* logName,
                                       const char* module)
    : _cmd(cmd), _report(logName), _module(module) {}

cider::harmony::ReportOpt CoverageMeasurment::operator()(
    const std::vector<cider::recorder::Action>& actions) {
  const auto script = getScript(actions);

  assert(cider::coverage::cleanCoverage(_cmd.covDir));

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

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
                                                     const char* module)
    : CoverageMeasurment(cmd, "stepper.txt", module) {}

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

}  // namespace harmony
}  // namespace cider
