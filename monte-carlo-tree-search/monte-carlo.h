// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"

namespace cider {
namespace mcts {

using ObjectiveFunction =
    std::function<ObjectiveValue(const std::vector<recorder::Action>&)>;

struct MonteCarloSettings final {
  const char* configName = "MCTS_NAN";
  size_t maxIter = 100;
  size_t maxRollback = 30;
  size_t maxDepth = 10;
  double ucb_C = 1.4142;
  ObjectiveFunction objFunc;
};

std::ostream& operator<<(std::ostream& os, const MonteCarloSettings& settings);

std::vector<recorder::Action> run_mcts(std::mt19937& gen,
                                       const MonteCarloSettings& settings,
                                       const std::vector<recorder::Action>& input);

}  // namespace mcts
}  // namespace cider
