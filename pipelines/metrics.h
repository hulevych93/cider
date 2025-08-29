// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "results.h"

namespace cider {
namespace pipelines {

struct Metrics final {
  std::string testCase;

  double avgOldCov = 0, stdOldCov = 0, varOldCov = 0;
  double avgNewCov = 0, stdNewCov = 0, varNewCov = 0;
  double avgOldCfg = 0, stdOldCfg = 0, varOldCfg = 0;
  double avgNewCfg = 0, stdNewCfg = 0, varNewCfg = 0;
  double avgOldLen = 0, stdOldLen = 0, varOldLen = 0;
  double avgNewLen = 0, stdNewLen = 0, varNewLen = 0;
  double avgCovReachLen = 0, stdCovReachLen = 0, varCovReachLen = 0;
  double avgTime = 0, stdTime = 0, varTime = 0;

  double compression = 0;
  double covDelta = 0;
  double cfgDelta = 0;
};

Metrics computeMetrics(const std::vector<Result>& results);

std::optional<size_t> computeCoverageReachedLength(
    const recorder::Actions& oldActions,
    const recorder::Actions& newActions,
    const std::string& libName,
    const cider::Cmd& cmd);

}  // namespace pipelines
}  // namespace cider
