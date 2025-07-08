// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning.h"

#include "agent-model/scenario.h"
#include "agent-model/utils.h"
#include "agent-q-learning/q-learning-agent.h"

#include <iostream>
#include <thread>

#include <assert.h>

namespace cider {
namespace agent_model {
namespace qlearning {

void prelearningSession(const QLearningSettings& settings,
                        const recorder::Actions& list,
                        ObjectiveFunction objFunc) {
  QLearningAgent& agent = QLearningAgent::get();

  RewardCounter rwCounter;
  LearningScenario scenario(rwCounter, Seed::instance().get(),
                            settings.maxStateDepth, list, objFunc);
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

void learningSession(const QLearningSettings& settings,
                     const recorder::Actions& list,
                     ObjectiveFunction objFunc,
                     RewardCounter& counter,
                     IResultsLogger& logger,
                     const std::function<void(const IAgent&)>& dump) {
  QLearningAgent& agent = QLearningAgent::get();

  for (int i = 0; i < settings.episodes; ++i) {

      const auto learningRate =
        settings.learningRate + (double(i) / settings.episodes) * 0.9f;
    const auto expRate = 0.8f - (double(i) / settings.episodes) * 0.8f;

    std::cout << "Episode: " << i << ", expRate: " << expRate << std::endl;

    if ((i % 100) == 0) {
      std::cout << "dump" << std::endl;
      dump(agent);
    }

    LearningScenario scenario(counter, Seed::instance().get(),
                              settings.maxStateDepth, list, objFunc);

    auto nextState = scenario.getCurrentState();

    float total_reward = 0.0f;
    ExponentialMovingAverage smoothLoss(0.5);

    const auto chooseValidAction = [&]() -> std::optional<recorder::Action> {
        size_t rollback = 0;
        while (true) {
            auto actionOpt = agent.chooseEGreedyAction(scenario, expRate);
            if (!actionOpt.has_value())
            {
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
      if(!actionOpt.has_value()) {
          break;
      }
      const auto action = actionOpt.value();

      scenario.add(action);
      nextState = scenario.getCurrentState();
      assert(!nextState.empty());

      const auto rewardOpt = scenario.getReward();
      assert(rewardOpt.has_value());

      const auto loss = agent.updateQValues(stateBeforeAction, nextState,
                                            action, rewardOpt.value(),
                                            learningRate, settings.discountFactor);
      smoothLoss.add_value(loss);
      total_reward += rewardOpt.value();
    }

    logger.logLoss(i, smoothLoss.get_average());
    logger.logReward(i, total_reward);
  }

  dump(agent);
}

}  // namespace qlearning
}  // namespace agent_model
}  // namespace cider
