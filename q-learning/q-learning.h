// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "agent.h"
#include "logger.h"
#include "scenario.h"

namespace cider {
namespace qleaning {

struct LearningSettings final {
  double learningRate = 0.1;
  double discountFactor = 0.9;
  size_t episodes = 50U;
  size_t maxRollback = 10U;
  size_t maxStateDepth = 10U;
  ObjectiveFunction objFunc;
};

std::ostream& operator<<(std::ostream& os, const LearningSettings& settings);

void prelearningSession(const LearningSettings& settings,
                        const QActionList& list,
                        QAgent& agent);

void learningSession(RewardCounter& counter,
                     const LearningSettings& settings,
                     const QActionList& list,
                     QAgent& agent,
                     IResultsLogger& logger,
                     const std::function<void()>& dump);

enum class GenerationStrategyType { Greedy, EGreedy, Boltzmann, Random };
enum class GenerationStopType { LimitActions, GreaterCoverage };

struct GenerationSettings final {
  const char* configName = nullptr;

  GenerationStopType stopType = GenerationStopType::GreaterCoverage;
  size_t limitActions = 450U;

  GenerationStrategyType strategy = GenerationStrategyType::Greedy;
  float epsilon = 0.1f;      // for ε-Greedy
  float temperature = 1.0f;  // for Boltzmann
  size_t maxRollback = 10U;
  size_t maxStateDepth = 3U;
  ObjectiveFunction objFunc;
};

std::ostream& operator<<(std::ostream& os, const GenerationSettings& settings);

bool gererationSession(const GenerationSettings& settings,
                       const QAgent& agent,
                       const QActionList& initial,
                       QActionList& out);

QAgent& getAgent(const std::string& path = "");

}  // namespace qleaning
}  // namespace cider
