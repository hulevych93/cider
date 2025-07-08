// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "sarsa-learning.h"

#include "agent-model/scenario.h"
#include "agent-model/utils.h"

#include <iostream>
#include <thread>

#include <assert.h>

namespace cider {
namespace agent_model {
namespace sarsa {

void prelearningSession(const SarsaLearningSettings& settings,
                        const recorder::Actions& list,
                        ObjectiveFunction objFunc) {
  SarsaLearningAgent& agent = SarsaLearningAgent::get();

  RewardCounter rwCounter;
  agent_model::LearningScenario scenario(rwCounter, Seed::instance().get(),
                                         settings.maxStateDepth, list, objFunc);

  auto nextState = scenario.getCurrentState();

  int index = 0U;
  while (!scenario.isOver()) {
    const auto stateBeforeAction = nextState;
    const auto& action = list[index];
    const auto& nextAction =
        (index + 1 < list.size()) ? list[index + 1] : recorder::Action{};

    ++index;

    scenario.add(action);
    nextState = scenario.getCurrentState();

    if (const auto rewardOpt = scenario.getReward()) {
      agent.updateQValues(stateBeforeAction, nextState, action, nextAction,
                          rewardOpt.value(), settings.learningRate,
                          settings.discountFactor);
    } else {
      throw std::logic_error{"failed to run on initial sequence."};
    }
  }
}

void learningSession(const SarsaLearningSettings& settings,
                     const recorder::Actions& list,
                     ObjectiveFunction objFunc,
                     RewardCounter& counter,
                     IResultsLogger& logger,
                     const std::function<void(const IAgent&)>& dump) {
  SarsaLearningAgent& agent = SarsaLearningAgent::get();

  for (int i = 0; i < settings.episodes; ++i) {
    const double learningRate =
        settings.learningRate + (double(i) / settings.episodes) * 0.9;
    const double expRate = 0.8 - (double(i) / settings.episodes) * 0.8;

    std::cout << "Episode: " << i << ", expRate: " << expRate << std::endl;

    if ((i % 100) == 0) {
      dump(agent);
    }

    agent_model::LearningScenario scenario(
        counter, Seed::instance().get(), settings.maxStateDepth, list, objFunc);

    ExponentialMovingAverage smoothLoss(0.5);
    float total_reward = 0.0f;

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

    auto actionOpt = chooseValidAction();
    if(!actionOpt.has_value()) {
        break;
    }

    auto action = actionOpt.value();
    auto state = scenario.getCurrentState();

    while (!scenario.isOver()) {

      scenario.add(action);
      const auto nextState = scenario.getCurrentState();

      auto actionOpt = chooseValidAction();
      if(!actionOpt.has_value()) {
          break;
      }
      const auto nextAction = actionOpt.value_or(recorder::Action{});

      const auto rewardOpt = scenario.getReward();
      assert(rewardOpt.has_value());

      const auto loss = agent.updateQValues(state, nextState,
                                            action, nextAction, rewardOpt.value(),
                                            learningRate, settings.discountFactor);
      smoothLoss.add_value(loss);
      total_reward += rewardOpt.value();

      state = nextState;
      action = nextAction;
    }

    logger.logLoss(i, smoothLoss.get_average());
    logger.logReward(i, total_reward);
  }

  dump(agent);
}

}  // namespace sarsa
}  // namespace agent_model
}  // namespace cider
