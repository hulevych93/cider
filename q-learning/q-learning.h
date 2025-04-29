// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "agent.h"
#include "scenario.h"

namespace cider {
namespace qleaning {

struct Settings final {
  double learningRate = 0.1;
  double discountFactor = 0.9;
  size_t episodes = 50U;
  ObjectiveFunction objFunc;
};

std::ostream& operator<<(std::ostream& os, const Settings& settings);

void learningSession(const Settings& settings,
                     const QActionList& list,
                     QValuesAgent& agent);

}  // namespace qleaning
}  // namespace cider
