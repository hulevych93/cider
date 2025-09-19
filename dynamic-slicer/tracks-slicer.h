// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"

namespace cider {
namespace dslicer {

using ObjectiveFunction =
    std::function<ObjectiveValue(const std::vector<recorder::Action>&)>;

using FineObjectiveFunc =
    std::function<FineObjectiveValue(const std::vector<recorder::Action>&)>;

struct BatchTracksDSlicingSettings final {
  const char* configName = "DSL-TR_NAN";
  ObjectiveFunction objFunc;
  FineObjectiveFunc fineObjFunc;
  double baseline = 0.0;
};

inline std::ostream& operator<<(std::ostream& os,
                                const BatchTracksDSlicingSettings& s) {
  return os;
}

std::vector<recorder::Action> run_d_slicing_fast_tracks(
    const BatchTracksDSlicingSettings& settings,
    const std::vector<recorder::Action>& actionSpace);

}  // namespace dslicer
}  // namespace cider
