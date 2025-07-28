// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "cfg_measurer.h"

#include <assert.h>

#include <filesystem>
#include <iostream>

#include "recorder/details/generator.h"

namespace cider {
namespace cfg_coverage {

CoverageMeasurment::CoverageMeasurment(const Cmd& cmd, const char* module)
    : _cmd(cmd), _module(module) {}

CfgCoverageOpt CoverageMeasurment::getReport(
    const std::vector<cider::recorder::Action>& actions) {
  const auto script = getScript(actions);
  if (script.empty()) {
    return std::nullopt;
  }

  const auto binaryPath = std::string{_cmd.binPath} + "_cfg";

  std::string jsonReport;
  const auto result = runScript(binaryPath, _cmd.workingDir, script,
                                [&](const char* data, std::size_t size) {
                                  jsonReport += std::string{data, size};
                                });
  if (result) {
    std::string jsonStr = readCoverageFromStream(jsonReport);
    assert(!jsonStr.empty());
    try {
      const auto rootReport = deserializeCovReport(jsonStr);
      assert(rootReport.has_value());
      if (rootReport->status) {
        if (m_logger) {
          m_logger->log(_index, rootReport.value());
        }
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
