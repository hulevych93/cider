// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/cfg_coverage.h"
#include "coverage/logger.h"

#include <fstream>

namespace cider {
namespace cfg_coverage {

using CfgCoverageOpt = std::optional<cfg_coverage::Coverage>;

struct CoverageMeasurment final {
  CoverageMeasurment(const Cmd& cmd, const char* module);

  void setLogger(const std::string& logDir, const std::string& fileName) {
    auto fileLog = std::make_unique<FileLogger>(logDir, fileName);
    m_logger = std::move(fileLog);
  }

  void setLogger(const std::shared_ptr<ICoverageLogger>& logger) {
    m_logger = logger;
  }

  CfgCoverageOpt getReport(const std::vector<cider::recorder::Action>& actions);

  CoveragePerAction getFineReport(
      const std::vector<cider::recorder::Action>& actions);

  double operator()(const std::vector<cider::recorder::Action>& actions) {
    const auto rootReport = getReport(actions);
    if (rootReport.has_value()) {
      return rootReport->getPercentage();
    }
    return 0.0f;
  }

  auto getObjValueFunc() {
    return [this](const std::vector<cider::recorder::Action>& actions)
               -> ObjectiveValue {
      const auto rootReport = getReport(actions);

      ObjectiveValue value;
      if (rootReport.has_value()) {
        value.coverage = rootReport->getPercentage();
        value.coveredTracks = rootReport->coveredTracks;
      }
      return value;
    };
  }

  auto getFastObjValueFunc() {
    return [this](const std::vector<cider::recorder::Action>& actions)
               -> FineObjectiveValue {
      const auto rootReport = getFineReport(actions);

      FineObjectiveValue value;
      if (!rootReport.empty()) {
        value.coverage = rootReport.back().getPercentage();
        value.fineCoveredTracks.reserve(rootReport.size());
        for (const auto& entry : rootReport) {
          value.fineCoveredTracks.emplace_back(entry.coveredTracks);
        }
      }
      return value;
    };
  }

  std::string getScript(const std::vector<cider::recorder::Action>& actions,
                        bool enableMarks = false) const;

 private:
  const Cmd& _cmd;
  std::shared_ptr<ICoverageLogger> m_logger;

 protected:
  mutable size_t _index = 1U;
  const char* _module;
};

}  // namespace cfg_coverage
}  // namespace cider
