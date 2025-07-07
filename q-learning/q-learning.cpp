// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning.h"

#include "q-learning/q-scenario.h"
#include "q-learning/qtable-agent.h"

#include <iostream>
#include <thread>

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
  os << "],_mxStDp[";
  os << settings.maxStateDepth;
  os << "]";
  return os;
}

void prelearningSession(const LearningSettings& settings,
                        const recorder::Actions& list,
                        QAgent& agent) {
  RewardCounter rwCounter;
  QScenario scenario(rwCounter, agent.getSeed(), settings.maxStateDepth, list,
                     settings.objFunc);
  auto nextState = scenario.getCurrentState();

  int index = 0U;
  while (!scenario.isOver()) {
    const auto stateBeforeAction = nextState;
    const auto& action = list[index++];
    if ((index % 50) == 0) {
      std::cout << "Index: " << index << std::endl;
    }

    scenario.add(action);
    nextState = scenario.getCurrentState();
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

void learningSession(RewardCounter& rwCounter,
                     const LearningSettings& settings,
                     const recorder::Actions& list,
                     QAgent& agent,
                     IResultsLogger& logger,
                     const std::function<void()>& dump) {
  int lowLosCounter = 0;
  for (int i = 0; i < settings.episodes; ++i) {
    const auto rl1 =
        settings.learningRate + (double(i) / settings.episodes) * 0.9f;
    const auto expRate = 0.8f - (double(i) / settings.episodes) * 0.8f;

    std::cout << "Episode: " << i << ", expRate: " << expRate << std::endl;

    if ((i % 100) == 0) {
      std::cout << "dump" << std::endl;
      dump();
    }

    QScenario scenario(rwCounter, agent.getSeed(), settings.maxStateDepth, list,
                       settings.objFunc);
    auto nextState = scenario.getCurrentState();
    recorder::Action lastAction;

    size_t rollbackCount = 0;
    size_t noActionCount = 0;

    float total_reward = 0.0f;
    float total_loss = 0.0f;
    ExponentialMovingAverage smoothLoss(0.5);

    int steps = 0;
    int index = 0U;
    while (true) {
      ++steps;
      const auto stateBeforeAction = nextState;
      const auto action = agent.chooseEGreedyAction(scenario, expRate);
      if (!action.has_value()) {
        noActionCount++;
        if (rollbackCount > settings.maxRollback) {
          std::cout << "No action" << std::endl;
          if (const auto rewardOpt = scenario.getReward()) {
            agent.updateQValues(stateBeforeAction, nextState, lastAction,
                                rewardOpt.value(), rl1,
                                settings.discountFactor);
          }
          break;
        }
        continue;
      }
      noActionCount = 0;

      lastAction = action.value();

      scenario.add(lastAction);
      nextState = scenario.getCurrentState();
      assert(!nextState.empty());

      if (const auto rewardOpt = scenario.getReward()) {
        const auto loss = agent.updateQValues(stateBeforeAction, nextState,
                                              lastAction, rewardOpt.value(),
                                              rl1, settings.discountFactor);
        total_loss += loss;
        smoothLoss.add_value(loss);
        total_reward += rewardOpt.value();
        rollbackCount = 0U;
        ++index;
      } else {
        scenario.rollback();

        nextState = stateBeforeAction;
        ++rollbackCount;

        std::cout << "rollback" << std::endl;
        if (rollbackCount > settings.maxRollback) {
          std::cout << "Max rollback" << std::endl;
          break;
        }
      }

      if (scenario.isOver()) {
        break;
      }
    }

    const float average_loss = total_loss / steps;
    std::cout << "Average Loss: " << average_loss << std::endl;

    logger.logLoss(i, smoothLoss.get_average());
    logger.logReward(i, total_reward);

    if (smoothLoss.get_average() < 0.0005) {
      lowLosCounter++;
      if (lowLosCounter > 10) {
        break;
      }
    } else {
      lowLosCounter = 0;
    }

    total_loss = 0.0f;
    steps = 0;
  }
}

QAgent& getAgent(const std::string& path) {
  static QTableAgent agent(path);
  return agent;
}

}  // namespace qleaning
}  // namespace cider
