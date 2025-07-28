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

  static SarsaLearningAgent& get() {
    if (Path.empty()) {
      throw std::logic_error{"agent path error"};
    }
    static SarsaLearningAgent agent(Path);
    return agent;
  }

  static void setPath(const std::string& path) { Path = path; }

  double updateQValues(const recorder::Actions& state,
                       const recorder::Actions& nextState,
                       const recorder::Action& action,
                       const recorder::Action& nextAction,
                       const double reward,
                       const double learningRate,
                       const double discount);

  static std::string Path;
};

}  // namespace sarsa
}  // namespace agent_model
}  // namespace cider
