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

std::vector<recorder::Action> run_d_slicing(
    const ObjectiveFunction& objFunc,
    const std::vector<recorder::Action>& input);

}  // namespace dslicer
}  // namespace cider
