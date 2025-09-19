// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"

#include "tracks-slicer.h"

#include <variant>

namespace cider {
namespace dslicer {

struct DSlicingSettings final {
  const char* configName = "DSL_NAN";
  ObjectiveFunction objFunc;
  double baseline = 0.0;
};

std::ostream& operator<<(std::ostream& os, const DSlicingSettings& s);

std::vector<recorder::Action> run_d_slicing(
    const DSlicingSettings& settings,
    const std::vector<recorder::Action>& input);

struct BatchDSlicingSettings final {
  const char* configName = "DSL-F_NAN";
  ObjectiveFunction objFunc;
  FineObjectiveFunc fineObjFunc;
  size_t batchSize = 5;
  double baseline = 0.0;
};

std::ostream& operator<<(std::ostream& os, const BatchDSlicingSettings& s);

std::vector<recorder::Action> run_d_slicing_batch(
    const BatchDSlicingSettings& settings,
    const std::vector<recorder::Action>& actionSpace);

struct BatchMultiPassDSlicingSettings final {
  const char* configName = "DSL-FM_NAN";
  ObjectiveFunction objFunc;
  FineObjectiveFunc fineObjFunc;
  double initialStepRatio = 0.2;
  size_t minimalGranularity = 1;
  double baseline = 0.0;
};

std::ostream& operator<<(std::ostream& os,
                         const BatchMultiPassDSlicingSettings& s);

std::vector<recorder::Action> run_d_slicing_batch_multipass(
    const BatchMultiPassDSlicingSettings& settings,
    const std::vector<recorder::Action>& actionSpace);

using DSLSettings = std::variant<DSlicingSettings,
                                 BatchDSlicingSettings,
                                 BatchMultiPassDSlicingSettings,
                                 BatchTracksDSlicingSettings>;

}  // namespace dslicer
}  // namespace cider
