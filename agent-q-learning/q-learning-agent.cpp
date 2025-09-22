// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning-agent.h"

#include <tlog.h>
#include <random>

namespace cider {
namespace agent_model {
namespace qlearning {

double QLearningAgent::updateQValues(const recorder::Actions& state,
                                     const recorder::Actions& nextState,
                                     const recorder::Action& action,
                                     const double reward,
                                     const double learningRate,
                                     const double discount) {
  auto& qValues = m_qtable[state];
  auto& qValue = qValues[action];

  double maxQValue = 0;
  const auto qNextValuesIter = m_qtable.find(nextState);
  if (qNextValuesIter != m_qtable.cend()) {
    const auto& qNextValues = qNextValuesIter->second;
    for (const auto& qNextValue : qNextValues) {
      maxQValue = std::max(maxQValue, qNextValue.second);
    }
  }

  tlog_info << "lr: " << learningRate << ", r: " << reward
            << ", mV: " << maxQValue << ", qv: " << qValue << " -> ";

  qValue += learningRate * (reward + discount * maxQValue - qValue);

  float target = reward + discount * maxQValue;
  float loss = 0.5f * (qValue - target) * (qValue - target);

  tlog_info << qValue << ", target:" << target << ", loss: " << loss
            << std::endl;

  return loss;
}

std::string QLearningAgent::Path;

}  // namespace qlearning
}  // namespace agent_model
}  // namespace cider
