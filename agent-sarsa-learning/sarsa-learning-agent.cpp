// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "sarsa-learning-agent.h"

#include <tlog.h>
#include <random>

namespace cider {
namespace agent_model {
namespace sarsa {

double SarsaLearningAgent::updateQValues(const recorder::Actions& state,
                                         const recorder::Actions& nextState,
                                         const recorder::Action& action,
                                         const recorder::Action& nextAction,
                                         const double reward,
                                         const double learningRate,
                                         const double discount) {
  auto& qValues = m_qtable[state];

  auto& qValue = qValues[action];
  const double nextQValue = m_qtable[nextState][nextAction];

  tlog_info << "lr: " << learningRate << ", r: " << reward
            << ", nextQ: " << nextQValue << ", qv: " << qValue << " -> ";

  qValue += learningRate * (reward + discount * nextQValue - qValue);

  float target = reward + discount * nextQValue;
  float loss = 0.5f * (qValue - target) * (qValue - target);

  tlog_info << qValue << ", target:" << target << ", loss: " << loss
            << std::endl;

  return loss;
}

std::string SarsaLearningAgent::Path;

}  // namespace sarsa
}  // namespace agent_model
}  // namespace cider
