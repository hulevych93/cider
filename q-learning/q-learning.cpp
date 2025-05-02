// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning.h"

namespace cider {
namespace qleaning {

std::ostream& operator<<(std::ostream& os, const LearningSettings& settings) {
  os << "QL_";
  os << "lr[";
  os << settings.learningRate;
  os << "]_df[";
  os << settings.discountFactor;
  os << "]_epds[";
  os << settings.episodes;
  os << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const GenerationSettings& settings) {
  os << "QL_GEN_";
  std::string strategyName;
  switch (settings.strategy) {
    case GenerationStrategyType::Greedy:
      strategyName = "Greedy";
      break;
    case GenerationStrategyType::EGreedy:
      strategyName = "E-Greedy";
      break;
    case GenerationStrategyType::Boltzmann:
      strategyName = "Boltzmann";
      break;
    default:
      strategyName = "Unknown";
  }

  os << "st[" << strategyName;
  os << "]_eps[" << settings.epsilon;
  os << "]_temp[" << settings.temperature;
  os << "]_maxSt[" << settings.maxSteps;
  os << " ]";
  return os;
}

void learningSession(const LearningSettings& settings,
                     const QActionList& list,
                     QValuesAgent& agent) {
  const auto episodes = settings.episodes;
  for (int i = 0; i < episodes; ++i) {
    const auto expRate = double(episodes - i) / episodes;

    Scenario scenario(list, settings.objFunc);
    auto nextState = scenario.toString();

    size_t rollbackCount = 0;
    while (!scenario.isOver()) {
      const auto stateBeforeAction = nextState;
      const auto action = agent.chooseEGreedyAction(scenario, expRate);

      scenario.add(action);
      nextState = scenario.toString();
      if (const auto rewardOpt = scenario.getReward()) {
        agent.updateQValues(stateBeforeAction, nextState, action,
                            rewardOpt.value(), settings.learningRate,
                            settings.discountFactor);
        rollbackCount = 0U;
      } else {
        scenario.rollback();
        nextState = stateBeforeAction;
        ++rollbackCount;
        if (rollbackCount > settings.maxRollback) {
          break;
        }
      }
    }
  }
}

bool gererationSession(const GenerationSettings& settings,
                       const QValuesAgent& agent,
                       const QActionList& initial,
                       QActionList& out) {
  out.clear();
  Scenario scenario(initial, settings.objFunc);

  for (size_t t = 0; t < settings.maxSteps; ++t) {
    QAction selected;

    switch (settings.strategy) {
      case GenerationStrategyType::Greedy:
        selected = agent.chooseGreedyAction(scenario);
        break;

      case GenerationStrategyType::EGreedy:
        selected = agent.chooseEGreedyAction(scenario, settings.epsilon);
        break;

      case GenerationStrategyType::Boltzmann:
        selected = agent.chooseBolzmanAction(scenario, settings.temperature);
        break;
    }

    scenario.add(selected);
    const auto rewardOpt = scenario.getReward();
    if (!rewardOpt.has_value()) {
      scenario.rollback();
    }
  }

  out = scenario.getCurrentState();

  return true;
}

}  // namespace qleaning
}  // namespace cider
