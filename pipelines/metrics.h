// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "results.h"

namespace cider {
namespace pipelines {

struct Metrics final {
    std::string testCase;
    double avgOldCov = 0, stdOldCov = 0;
    double avgNewCov = 0, stdNewCov = 0;
    double avgOldCfg = 0, stdOldCfg = 0;
    double avgNewCfg = 0, stdNewCfg = 0;
    double avgOldLen = 0, stdOldLen = 0;
    double avgNewLen = 0, stdNewLen = 0;
    double avgTime = 0, stdTime = 0;

    double compression = 0;
    double covDelta = 0;
    double cfgDelta = 0;
    double effScore = 0;
};

Metrics computeMetrics(const std::vector<Result>& results);

}  // namespace pipelines
}  // namespace cider
