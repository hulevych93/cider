// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning.h"

namespace cider {
namespace qleaning {

std::ostream& operator<<(std::ostream& os, const Settings& settings) {
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

void learningSession(const Settings& settings,
                     const QActionList& list,
                     QValuesAgent& agent) {
  const auto episodes = settings.episodes;
  for (int i = 0; i < episodes; ++i) {
    const auto expRate = double(episodes - i) / episodes;

    Scenario scenario(list, settings.objFunc);
    auto nextState = scenario.toString();

    while (!scenario.isOver()) {
      const auto stateBeforeAction = nextState;
      const auto action = agent.chooseAction(scenario, expRate);

      scenario.add(action);
      nextState = scenario.toString();
      if (const auto rewardOpt = scenario.getReward()) {
        agent.updateQValues(stateBeforeAction, nextState, action,
                            rewardOpt.value(), settings.learningRate,
                            settings.discountFactor);
      } else {
        scenario.rollback();
        nextState = stateBeforeAction;
      }
    }
  }
}

}  // namespace qleaning
}  // namespace cider
