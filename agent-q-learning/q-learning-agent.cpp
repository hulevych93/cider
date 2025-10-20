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

  double maxQValue = 0.0;
  if (auto it = m_qtable.find(nextState); it != m_qtable.end()) {
    for (const auto& [_, qNext] : it->second)
      maxQValue = std::max(maxQValue, qNext);
  }

  const double target = reward + discount * maxQValue;
  const double tdError = target - qValue;

  tlog_info << "lr:" << learningRate << ", r:" << reward << ", mV:" << maxQValue
            << ", qv:" << qValue << " -> ";

  qValue += learningRate * tdError;

  const size_t L = state.size();
  const double lambda = 0.8;
  for (size_t k = 1; k < L; ++k) {
    recorder::Actions suffix(state.end() - static_cast<long>(k), state.end());
    auto& qValsK = m_qtable[suffix];
    auto& qvK = qValsK[action];
    const double decay = std::pow(lambda, static_cast<double>(L - k));
    qvK += learningRate * decay * tdError;
  }

  const double loss = 0.5 * tdError * tdError;
  tlog_info << qValue << ", target:" << target << ", loss:" << loss
            << std::endl;

  return loss;
}

std::string QLearningAgent::Path;

SuffixLogger QLearningAgent::Logger;

}  // namespace qlearning
}  // namespace agent_model
}  // namespace cider
