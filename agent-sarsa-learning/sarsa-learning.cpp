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
                        ObjectiveFunction objFunc,
                        IRewardLogger& rwLogger,
                        ILossLogger& lossLogger) {
  SarsaLearningAgent& agent = SarsaLearningAgent::get();

  for (int i = 0; i < settings.prelearningEpisodes; ++i) {
    RewardCounter rwCounter;
    agent_model::LearningScenario scenario(rwCounter, Seed::instance().get(),
                                           settings.maxStateDepth, list,
                                           objFunc);

    auto nextState = scenario.getCurrentState();

    ExponentialMovingAverage smoothLoss(0.5);
    float total_reward = 0.0f;

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
        const auto loss = agent.updateQValues(
            stateBeforeAction, nextState, action, nextAction, rewardOpt.value(),
            settings.initialLearningRate, settings.discountFactor);

        smoothLoss.add_value(loss);
        total_reward += rewardOpt.value();
      } else {
        throw std::logic_error{"failed to run on initial sequence."};
      }

      lossLogger.log(index, smoothLoss.get_average());
      rwLogger.log(index, total_reward);
    }
  }
}

void learningSession(const SarsaLearningSettings& settings,
                     const recorder::Actions& list,
                     ObjectiveFunction objFunc,
                     RewardCounter& counter,
                     IRewardLogger& rwLogger,
                     ILossLogger& lossLogger,
                     ICoverageLogger& covLogger,
                     const std::function<void(const IAgent&)>& dump) {
  SarsaLearningAgent& agent = SarsaLearningAgent::get();

  const auto objValue = objFunc(list);
  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
    covLogger.set(objValue.coverage);
  } else {
    throw std::logic_error{"Bad initial script."};
  }

  int coverageCounter = 0;
  for (int i = 0; i < settings.episodes; ++i) {
    const auto learningRate =
        linearDecay(settings.initialLearningRate, settings.finalLearningRate,
                    settings.episodes, i);
    const auto expRate = linearDecay(0.9, 0.1, settings.episodes, i);

    std::cout << "Episode: " << i << ", expRate: " << expRate << std::endl;

    agent_model::LearningScenario scenario(
        counter, Seed::instance().get(), settings.maxStateDepth, list, objFunc);

    ExponentialMovingAverage smoothLoss(0.5);
    float total_reward = 0.0f;

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

    auto actionOpt = chooseValidAction();
    if (!actionOpt.has_value()) {
      break;
    }

    auto action = actionOpt.value();
    auto state = scenario.getCurrentState();

    while (!scenario.isOver()) {
      scenario.add(action);
      const auto nextState = scenario.getCurrentState();

      auto actionOpt = chooseValidAction();
      if (!actionOpt.has_value()) {
        break;
      }
      const auto nextAction = actionOpt.value_or(recorder::Action{});

      const auto rewardOpt = scenario.getReward();
      assert(rewardOpt.has_value());

      const auto loss = agent.updateQValues(
          state, nextState, action, nextAction, rewardOpt.value(), learningRate,
          settings.discountFactor);
      smoothLoss.add_value(loss);
      total_reward += rewardOpt.value();

      state = nextState;
      action = nextAction;
    }

    lossLogger.log(i, smoothLoss.get_average());
    rwLogger.log(i, total_reward);
    const auto coverage = scenario.getCoverage(true);
    covLogger.log(i, coverage);

    if (coverage >= objValue.coverage) {
      coverageCounter++;
      if (coverageCounter > settings.coverageConvergenceCounter) {
        break;
      }
    } else {
      coverageCounter = 0;
    }
  }

  dump(agent);
}

}  // namespace sarsa
}  // namespace agent_model
}  // namespace cider
