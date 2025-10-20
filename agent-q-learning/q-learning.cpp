// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning.h"

#include "agent-model/scenario.h"
#include "agent-model/utils.h"
#include "agent-q-learning/q-learning-agent.h"

#include "mathplot-log/monitoring/q-learning-cov-ep-plot.h"

#include <tlog.h>
#include <thread>

#include <assert.h>

namespace cider {
namespace agent_model {
namespace qlearning {

void prelearningSession(const QLearningSettings& settings,
                        const recorder::Actions& list,
                        ObjectiveFunction objFunc,
                        FineObjectiveFunction fineObjFunc,
                        IRewardLogger& rwLogger,
                        ILossLogger& lossLogger) {
  QLearningAgent& agent = QLearningAgent::get();

  for (int i = 0; i < settings.prelearningEpisodes; ++i) {
    RewardCounter rwCounter;
    LearningScenario scenario(settings.rewardShaping, rwCounter,
                              Seed::instance().get(), settings.maxStateDepth,
                              list, objFunc, fineObjFunc);
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
            settings.initialLearningRate, settings.discountFactor);

        smoothLoss.add_value(loss);
        total_reward += rewardOpt.value();
      } else {
        throw std::logic_error{"failed to run on initial sequence."};
      }
    }

    lossLogger.log(i, smoothLoss.get_average());
    rwLogger.log(i, total_reward);
  }
}

void learningSession(const QLearningSettings& settings,
                     const recorder::Actions& list,
                     ObjectiveFunction objFunc,
                     FineObjectiveFunction fineObjFunc,
                     RewardCounter& counter,
                     IRewardLogger& rwLogger,
                     ILossLogger& lossLogger,
                     ICoverageLogger& covLogger,
                     const std::function<void(const IAgent&)>& dump) {
  QLearningAgent& agent = QLearningAgent::get();

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

    tlog_info << "Episode: " << i << ", expRate: " << expRate
              << ", LR: " << learningRate << std::endl;

    LearningScenario scenario(settings.rewardShaping, counter,
                              Seed::instance().get(), settings.maxStateDepth,
                              list, objFunc, fineObjFunc);

    auto nextState = scenario.getCurrentState();

    float total_reward = 0.0f;
    ExponentialMovingAverage smoothLoss(0.01);

    const auto chooseValidAction = [&]() -> std::optional<recorder::Action> {
      size_t rollback = 0;
      while (true) {
        std::optional<recorder::Action> actionOpt;

        switch (settings.strategy) {
          case ActionMakerStrategyType::Greedy:
            actionOpt = agent.chooseGreedyAction(scenario);
            break;
          case ActionMakerStrategyType::EGreedy:
            actionOpt = agent.chooseEGreedyAction(scenario, expRate);
            break;
          case ActionMakerStrategyType::Boltzmann:
            actionOpt =
                agent.chooseBoltzmannAction(scenario, settings.temperature);
            break;
          case ActionMakerStrategyType::BoltzmannWithOpeners:
            actionOpt = agent.chooseBoltzmannWithOpenersAction(
                scenario, settings.temperature, settings.lambda,
                settings.top_k);
            break;
        }

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

    lossLogger.log(i, smoothLoss.get_average());
    rwLogger.log(i, total_reward);

    const auto coverage = scenario.getCoverage(true);
    covLogger.log(i, coverage);

    if (coverage >= objValue.coverage) {
      coverageCounter++;
      if (coverageCounter > 1000) {
        break;
      }
    } else {
      coverageCounter = 0;
    }
  }

  dump(agent);
}

}  // namespace qlearning
}  // namespace agent_model
}  // namespace cider
