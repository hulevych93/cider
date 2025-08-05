// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"

namespace cider {
namespace greedy_r {

using ObjectiveFunction =
    std::function<ObjectiveValue(const std::vector<recorder::Action>&)>;

struct GreedyRSettings final {
  const char* configName = "GREEDY_R_NAN";
  size_t maxZeroGain = 10;
  size_t top_k = 3;
  ObjectiveFunction objFunc;
};

std::ostream& operator<<(std::ostream& os, const GreedyRSettings& settings);

std::vector<recorder::Action> run_greedy_r(
    std::mt19937& gen,
    const GreedyRSettings& settings,
    const std::vector<recorder::Action>& input);

}  // namespace greedy_r
}  // namespace cider
