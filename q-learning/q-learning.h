// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"
#include "coverage/measurer.h"

#include <random>

namespace cider {
namespace qleaning {

using MeassureCallback = std::function<cider::coverage::ReportOpt(
    const std::vector<recorder::Action>&)>;

struct Settings final {
  int populationSize = 10;
  double Pa = 0.25;
  size_t maxIterationsWithoutUpdates = 500U;
  MeassureCallback meassure;
};

struct Nest final {
  std::vector<recorder::Action> actions;
  coverage::RootReport cov;
};

}  // namespace qleaning
}  // namespace cider
