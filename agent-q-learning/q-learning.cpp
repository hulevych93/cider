// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning.h"

#include "agent-model/scenario.h"
#include "agent-model/utils.h"
#include "agent-q-learning/q-learning-agent.h"

#include "mathplot-log/monitoring/q-learning-cov-ep-plot.h"

#include <iostream>
#include <thread>

#include <assert.h>

namespace cider {
namespace agent_model {
namespace qlearning {

void prelearningSession(const QLearningSettings& settings,
                        const recorder::Actions& list,
                        ObjectiveFunction objFunc,
                        IResultsLogger& logger) {
  QLearningAgent& agent = QLearningAgent::get();

  for (int i = 0; i < settings.prelearningEpisodes; ++i) {
    RewardCounter rwCounter;
    LearningScenario scenario(rwCounter, Seed::instance().get(),
                              settings.maxStateDepth, list, objFunc);
    auto nextState = scenario.getCurrentState();

    float total_reward = 0.0f;
    ExponentialMovingAverage smoothLoss(0.5);

    int index = 0U;
    while (!scenario.isOver()) {
      const auto stateBeforeAction = nextState;
      const auto& action = list[index++];

      scenario.add(action);
      nextState = scenario.getCurrentState();
      assert(!nextState.empty());

      if (const auto rewardOpt = scenario.getReward()) {
        const auto loss = agent.updateQValues(
            stateBeforeAction, nextState, action, rewardOpt.value(),
            settings.learningRate, settings.discountFactor);

        smoothLoss.add_value(loss);
        total_reward += rewardOpt.value();
      } else {
        throw std::logic_error{"failed to run on initial sequence."};
      }
    }

    logger.logLoss(i, smoothLoss.get_average());
    logger.logReward(i, total_reward);
  }
}

void learningSession(const QLearningSettings& settings,
                     const recorder::Actions& list,
                     ObjectiveFunction objFunc,
                     RewardCounter& counter,
                     IResultsLogger& logger,
                     ICovLogger& covLogger,
                     const std::function<void(const IAgent&)>& dump) {
  QLearningAgent& agent = QLearningAgent::get();

  const auto objValue = objFunc(list);
  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
      covLogger.set(objValue.coverage);
  } else {
      throw std::logic_error{"Bad initial script."};
  }

  for (int i = 0; i < settings.episodes; ++i) {
    const auto learningRate =
        settings.learningRate + (double(i) / settings.episodes) * 0.5f;
    const auto expRate = 0.9f - ((double(i) / settings.episodes) * 0.8f);

    std::cout << "Episode: " << i << ", expRate: " << expRate << std::endl;

    if ((i % 100) == 0) {
      std::cout << "dump" << std::endl;
      dump(agent);
    }

    LearningScenario scenario(counter, Seed::instance().get(),
                              settings.maxStateDepth, list, objFunc);

    auto nextState = scenario.getCurrentState();

    float total_reward = 0.0f;
    ExponentialMovingAverage smoothLoss(0.01);

    const auto chooseValidAction = [&]() -> std::optional<recorder::Action> {
      size_t rollback = 0;
      while (true) {
        auto actionOpt = agent.chooseEGreedyAction(scenario, expRate);
        if (!actionOpt.has_value()) {
          rollback++;
          if (rollback > settings.maxRollback) {
            return std::nullopt;
          }
          continue;
        }

        scenario.add(actionOpt.value());
        if (!scenario.isValid(false)) {
          scenario.rollback();
          rollback++;
          if (rollback > settings.maxRollback) {
            return std::nullopt;
          }
          continue;
        }

        scenario.rollback();
        return actionOpt;
      }
    };

    while (!scenario.isOver()) {
      const auto stateBeforeAction = nextState;
      const auto actionOpt = chooseValidAction();
      if (!actionOpt.has_value()) {
        break;
      }
      const auto action = actionOpt.value();

      scenario.add(action);
      nextState = scenario.getCurrentState();
      assert(!nextState.empty());

      const auto rewardOpt = scenario.getReward();
      assert(rewardOpt.has_value());

      const auto loss = agent.updateQValues(
          stateBeforeAction, nextState, action, rewardOpt.value(), learningRate,
          settings.discountFactor);
      smoothLoss.add_value(loss);
      total_reward += rewardOpt.value();
    }

    logger.logLoss(i, smoothLoss.get_average());
    logger.logReward(i, total_reward);

    covLogger.log(i, scenario.getCoverage());

    if ((i % 100) == 0) {
      logger.save();
    }
  }

  dump(agent);
}

}  // namespace qlearning
}  // namespace agent_model
}  // namespace cider
