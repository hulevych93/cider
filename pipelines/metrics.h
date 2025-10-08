// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "results.h"

namespace cider {
namespace pipelines {

struct Metrics final {
  std::string testCase;

  // coverage
  double avgOldCov = 0, stdOldCov = 0, varOldCov = 0;
  double avgNewCov = 0, stdNewCov = 0, varNewCov = 0;
  double covDelta = 0;

  // cfg coverage
  double avgOldCfg = 0, stdOldCfg = 0, varOldCfg = 0;
  double avgNewCfg = 0, stdNewCfg = 0, varNewCfg = 0;
  double cfgDelta = 0;

  // length
  double avgOldLen = 0, stdOldLen = 0, varOldLen = 0;
  double avgNewLen = 0, stdNewLen = 0, varNewLen = 0;
  double avgCovReachLen = 0, stdCovReachLen = 0, varCovReachLen = 0;

  // execution times
  double avgOldTime = 0, stdOldTime = 0, varOldTime = 0;
  double avgNewTime = 0, stdNewTime = 0, varNewTime = 0;
  double avgTotalTime = 0, stdTotalTime = 0, varTotalTime = 0;

  // derived metrics
  double compression = 0;  // newLen / oldLen (тільки для сценаріїв з retention)
  double stdCompression = 0;
  double varCompression = 0;

  double timeReduction = 0;  // (oldTime - newTime)/oldTime
  double stdTimeReduction = 0;
  double varTimeReduction = 0;

  double jScore = 0;  // α*C - β*Red - λ*L/Lmax

  // retention
  size_t retainedCount = 0;
  size_t totalCount = 0;
  double coverageRetentionRate = 0;
};

void getCompression(const std::string& methodName,
                    const std::string& libName,
                    const Result& result,
                    const std::function<void(unsigned long, double)> handler);

void getExecutionTimeUpToCovReach(
    const cider::Cmd& cmd,
    const std::string& methodName,
    const std::string& libName,
    const Result& result,
    unsigned long covReachLen,
    const std::function<void(unsigned long elapsedMcs)> handler);

Metrics computeMetrics(const cider::Cmd& cmd,
                       const std::string& methodName,
                       const std::string& libName,
                       const std::vector<Result>& results,
                       double alpha = 1.0,
                       double beta = 1.0,
                       double lambda = 1.0);

std::optional<size_t> computeCoverageReachedLength(
    const recorder::Actions& oldActions,
    const recorder::Actions& newActions,
    const std::string& libName,
    const cider::Cmd& cmd);

}  // namespace pipelines
}  // namespace cider
