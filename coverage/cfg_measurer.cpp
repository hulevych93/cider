// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "cfg_measurer.h"

#include <assert.h>

#include <iostream>

#include "recorder/details/generator.h"

namespace cider {
namespace cfg_coverage {

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

CfgCoverageOpt CoverageMeasurment::operator()(
    const std::vector<cider::recorder::Action>& actions) {
  const auto script = getScript(actions);
  if (script.empty()) {
    return std::nullopt;
  }

  std::string jsonReport;
  const auto result = runScript(_cmd.binPath, _cmd.workingDir, script,
                                [&](const char* data, std::size_t size) {
                                  jsonReport += std::string{data, size};
                                });
  if (result) {
    std::string jsonStr = readCoverageJsonFromStream(jsonReport);
    assert(!jsonStr.empty());
    try {
      const auto rootReport = parseJsonCovReport(jsonStr);
      assert(rootReport.has_value());
      if (rootReport->status) {
        printTableEntry(_report, _index, rootReport.value());
        return rootReport.value();
      }
    } catch (...) {
      std::cerr << "Failed to parse coverage JSON!\n";
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

}  // namespace cfg_coverage
}  // namespace cider
