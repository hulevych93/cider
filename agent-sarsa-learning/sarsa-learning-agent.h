// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "agent-model/qtable-agent.h"

namespace cider {
namespace agent_model {
namespace sarsa {

class SarsaLearningAgent : public agent_model::QTableAgent {
  using QTableAgent::QTableAgent;

 public:
  ~SarsaLearningAgent() override = default;

  static SarsaLearningAgent& get(const std::string& path = "") {
    static SarsaLearningAgent agent(path);
    return agent;
  }

  double updateQValues(const recorder::Actions& state,
                       const recorder::Actions& nextState,
                       const recorder::Action& action,
                       const recorder::Action& nextAction,
                       const double reward,
                       const double learningRate,
                       const double discount);
};

}  // namespace sarsa
}  // namespace agent_model
}  // namespace cider
