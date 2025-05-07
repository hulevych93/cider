// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "agent.h"
#include "scenario.h"

namespace cider {
namespace qleaning {

struct LearningSettings final {
  double learningRate = 0.1;
  double discountFactor = 0.9;
  size_t episodes = 50U;
  size_t maxRollback = 10U;
  ObjectiveFunction objFunc;
};

std::ostream& operator<<(std::ostream& os, const LearningSettings& settings);

void learningSession(const LearningSettings& settings,
                     const QActionList& list,
                     QValuesAgent& agent);

enum class GenerationStrategyType { Greedy, EGreedy, Boltzmann };

struct GenerationSettings final {
  GenerationStrategyType strategy = GenerationStrategyType::Greedy;
  float epsilon = 0.1f;      // for ε-Greedy
  float temperature = 1.0f;  // for Boltzmann
  size_t maxRollback = 10U;
  ObjectiveFunction objFunc;
};

std::ostream& operator<<(std::ostream& os, const GenerationSettings& settings);

bool gererationSession(const GenerationSettings& settings,
                       const QValuesAgent& agent,
                       const QActionList& initial,
                       QActionList& out);

}  // namespace qleaning
}  // namespace cider
