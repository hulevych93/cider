// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"

#include <variant>

namespace cider {
namespace dslicer {

using ObjectiveFunction =
    std::function<ObjectiveValue(const std::vector<recorder::Action>&)>;

using FineObjectiveFunc =
    std::function<FineObjectiveValue(const std::vector<recorder::Action>&)>;

struct DSlicingSettings final {
  const char* configName = "DSL_NAN";
  ObjectiveFunction objFunc;
};

std::vector<recorder::Action> run_d_slicing(
    const DSlicingSettings& settings,
    const std::vector<recorder::Action>& input);

struct FastDSlicingSettings final {
  const char* configName = "DSL-F_NAN";
  ObjectiveFunction objFunc;
  FineObjectiveFunc fineObjFunc;
  size_t checkStep = 5;
};

std::vector<recorder::Action> run_d_slicing_fast_checked(
    const FastDSlicingSettings& settings,
    const std::vector<recorder::Action>& actionSpace,
    double baseline = 0);

struct FastMultiPassDSlicingSettings final {
  const char* configName = "DSL-FM_NAN";
  ObjectiveFunction objFunc;
  FineObjectiveFunc fineObjFunc;
  double initialStepRatio = 0.2;
  size_t minimalGranularity = 1;
  double baseline = 0.0;
};

std::vector<recorder::Action> run_d_slicing_fast_multipass(
    const FastMultiPassDSlicingSettings& settings,
    const std::vector<recorder::Action>& actionSpace);

using DSLSettings = std::variant<DSlicingSettings,
                                 FastDSlicingSettings,
                                 FastMultiPassDSlicingSettings>;

}  // namespace dslicer
}  // namespace cider
