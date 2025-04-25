// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning.h"

#include "scenario.h"

namespace cider {
namespace qleaning {

void learningSession(cider::coverage::CoverageMeasurment& meassurer,
                     const QActionList& list,
                     QValuesAgent& agent,
                     const int episodes) {
  for (int i = 0; i < episodes; ++i) {
    const auto expRate = double(episodes - i) / episodes;

    Scenario scenario(
        list,
        std::bind(&cider::coverage::CoverageMeasurment::operator(),
                  std::ref(meassurer), std::placeholders::_1));
    auto nextState = scenario.toString();

    while (!scenario.isOver()) {
      const auto stateBeforeAction = nextState;
      const auto action = agent.chooseAction(scenario, expRate);

      scenario.add(action);
      nextState = scenario.toString();
      if(const auto rewardOpt = scenario.getReward())
      {
          agent.updateQValues(stateBeforeAction, nextState, action,
                              rewardOpt.value(), LEARNING_RATE, DISCOUNT_FACTOR);
      } else {
          scenario.rollback();
          nextState = stateBeforeAction;
      }
    }
  }
}

}  // namespace qleaning
}  // namespace cider
