// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "cfg_measurer.h"

#include <assert.h>

#include <filesystem>

#include "recorder/details/generator.h"
#include "scripting/runner.h"

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
  const auto result =
      scripting::runScript(binaryPath, _cmd.workingDir, script,
                           [&](const char* data, std::size_t size) {
                             jsonReport += std::string{data, size};
                           });
  if (result) {
    std::string jsonStr = readCoverageFromStream(jsonReport);

    try {
      const auto rootReport = deserializeCovReport(jsonStr);
      if (rootReport.has_value()) {
        if (rootReport->status) {
          if (m_logger) {
            m_logger->log(_index, rootReport.value());
          }
          return rootReport.value();
        }
      }
    } catch (...) {
      std::cerr << "Failed to parse coverage JSON!\n";
    }
  }

  return std::nullopt;
}

CoveragePerAction CoverageMeasurment::getFineReport(
    const std::vector<cider::recorder::Action>& actions) {
  const auto script = getScript(actions, true);
  if (script.empty()) {
    return {};
  }

  const auto binaryPath = std::string{_cmd.binPath} + "_fine";

  std::string jsonReport;
  const auto result =
      scripting::runScript(binaryPath, _cmd.workingDir, script,
                           [&](const char* data, std::size_t size) {
                             jsonReport += std::string{data, size};
                           });
  // if (result) {
  std::string jsonStr = readCoverageFromStream(jsonReport);

  try {
    return deserializeCovPerActReport(jsonStr);
  } catch (...) {
    std::cerr << "Failed to parse coverage JSON!\n";
  }
  // }

  return {};
}

std::string CoverageMeasurment::getScript(
    const std::vector<cider::recorder::Action>& actions,
    bool enableMarks) const {
  auto generator =
      cider::recorder::makeLuaGenerator(_module, enableMarks, actions.size());
  ++_index;
  return cider::recorder::generateScript(generator, actions, 999999U);
}

}  // namespace cfg_coverage
}  // namespace cider
