// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "agent.h"
#include "scenario.h"

namespace cider {
namespace qleaning {

const double LEARNING_RATE = 0.05;
const double DISCOUNT_FACTOR = 0.85;

void learningSession(const ObjectiveFunction& objFunc,
                     const QActionList& list,
                     QValuesAgent& agent,
                     const int episodes);

}  // namespace qleaning
}  // namespace cider
