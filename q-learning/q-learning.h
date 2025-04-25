// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"
#include "coverage/measurer.h"

#include "agent.h"

namespace cider {
namespace qleaning {

const double LEARNING_RATE = 0.05;
const double DISCOUNT_FACTOR = 0.8;

void learningSession(cider::coverage::CoverageMeasurment& meassurer,
                     const QActionList& list,
                     QValuesAgent& agent,
                     const int episodes);

}  // namespace qleaning
}  // namespace cider
