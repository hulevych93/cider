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

using FineObjectiveFunction =
    std::function<FineObjectiveValue(const std::vector<recorder::Action>&)>;

struct GreedyRTracksSettings final {
  const char* configName = "GREEDY_R_TRACKS_NAN";
  double temperature = 1.5;
  size_t top_k = 3;
  double lambda = 0.8;
  ObjectiveFunction objFunc;
  FineObjectiveFunction fineObjFunc;
  std::vector<double>* openers = nullptr;
  double baseline = 0.0;
};

std::ostream& operator<<(std::ostream& os,
                         const GreedyRTracksSettings& settings);

std::vector<recorder::Action> run_greedy_r_tracks(
    std::mt19937& gen,
    const GreedyRTracksSettings& settings,
    const std::vector<recorder::Action>& input);

}  // namespace greedy_r
}  // namespace cider
