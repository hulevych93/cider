// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "agent.h"
#include "scenario.h"

namespace cider {
namespace qleaning {

void learningSession(const ObjectiveFunction& objFunc,
                     const QActionList& list,
                     QValuesAgent& agent,
                     int episodes,
                     double learningRate,
                     double discountFactor);

}  // namespace qleaning
}  // namespace cider
