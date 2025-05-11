// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning.h"

#include <iostream>

#include <assert.h>

#include "utils.h"

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
  os << "]_maxSt[" << settings.maxRollback;
  os << " ]";
  return os;
}

void prelearningSession(const LearningSettings& settings,
                        const QActionList& list,
                        QValuesAgent& agent) {
  Scenario scenario(agent.getSeed(), settings.maxStateDepth, list,
                    settings.objFunc);
  auto nextState = scenario.toString();

  int index = 0U;
  while (!scenario.isOver()) {
    const auto stateBeforeAction = nextState;
    const auto& action = list[index++];
    if ((index % 50) == 0) {
      std::cout << "Index: " << index << std::endl;
    }

    scenario.add(action);
    nextState = scenario.toString();
    assert(!nextState.empty());
    if (const auto rewardOpt = scenario.getReward()) {
      agent.updateQValues(stateBeforeAction, nextState, action,
                          rewardOpt.value(), settings.learningRate,
                          settings.discountFactor);
    } else {
      throw std::logic_error{"failed to run on initial sequence."};
    }
  }
}

void learningSession(const LearningSettings& settings,
                     const QActionList& list,
                     QValuesAgent& agent) {
  EpsilonGreedyAdaptor epsAdaptor(1, 0.999, 0.04, 3);

  const auto episodes = settings.episodes;
  for (int i = 0; i < episodes; ++i) {
    std::cout << "Episode: " << i << ", expRate: " << epsAdaptor.get_epsilon()
              << std::endl;

    Scenario scenario(agent.getSeed(), settings.maxStateDepth, list,
                      settings.objFunc);
    auto nextState = scenario.toString();

    size_t rollbackCount = 0;
    size_t failCounter = 0;

    float total_reward = 0.0f;
    float total_loss = 0.0f;
    ExponentialMovingAverage smoothLoss(0.1);
    int steps = 0;

    int index = 0U;
    while (!scenario.isOver()) {
      ++steps;
      const auto stateBeforeAction = nextState;
      const auto actionOpt =
          agent.chooseEGreedyAction(scenario, epsAdaptor.get_epsilon());
      if (!actionOpt.has_value()) {
        std::cout << "No action" << std::endl;
        break;
      }
      const auto action = actionOpt.value();
      scenario.add(action);
      nextState = scenario.toString();
      assert(!nextState.empty());

      if (const auto rewardOpt = scenario.getReward()) {
        if (rewardOpt.value() > 0) {
          const auto loss = agent.updateQValues(
              stateBeforeAction, nextState, action, rewardOpt.value(),
              settings.learningRate, settings.discountFactor);
          total_loss += loss;
          smoothLoss.add_value(loss);
          total_reward += rewardOpt.value();
          rollbackCount = 0U;
          ++index;
          if ((index % 50) == 0) {
            std::cout << "Index: " << index << ", Fails: " << failCounter
                      << std::endl;
          }
          continue;
        }

        scenario.rollbackAndDrop();
        nextState = stateBeforeAction;
        continue;
      }

      scenario.rollback();
      nextState = stateBeforeAction;
      ++rollbackCount;
      failCounter++;
      if (rollbackCount > settings.maxRollback) {
        std::cout << "Max rollback" << std::endl;
        break;
      }
    }

    float average_loss = total_loss / steps;
    std::cout << "Average Loss: " << average_loss << std::endl;

    epsAdaptor.adapt(average_loss);

    agent.getLogger().logLoss(smoothLoss.get_average());

    total_loss = 0.0f;
    steps = 0;
  }
}

bool gererationSession(const GenerationSettings& settings,
                       const QValuesAgent& agent,
                       const QActionList& initial,
                       QActionList& out) {
  out.clear();

  std::random_device rd;
  std::mt19937 gen(rd());
  Scenario scenario(gen, settings.maxStateDepth, initial, settings.objFunc);

  size_t rollbackCount = 0;

  while (!scenario.isOver()) {
    std::optional<QAction> selectedOpt;

    switch (settings.strategy) {
      case GenerationStrategyType::Greedy:
        selectedOpt = agent.chooseGreedyAction(scenario);
        break;
      case GenerationStrategyType::EGreedy:
        selectedOpt = agent.chooseEGreedyAction(scenario, settings.epsilon);
        break;
      case GenerationStrategyType::Boltzmann:
        selectedOpt = agent.chooseBolzmanAction(scenario, settings.temperature);
        break;
    }

    if (!selectedOpt.has_value()) {
      std::cout << "No selected action" << std::endl;
      break;
    }
    const auto selected = selectedOpt.value();

    scenario.add(selected);
    const auto rewardOpt = scenario.getReward();
    if (rewardOpt.has_value()) {
      rollbackCount = 0U;

    } else {
      scenario.rollback();
      ++rollbackCount;
      if (rollbackCount > settings.maxRollback) {
        std::cout << "Max rollback" << std::endl;
        break;
      }
    }
  }

  out = scenario.getCurrentState();

  return true;
}

}  // namespace qleaning
}  // namespace cider
