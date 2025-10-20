// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "agent-model/qtable-agent.h"

namespace cider {
namespace agent_model {
namespace qlearning {

class QLearningAgent : public agent_model::QTableAgent {
  using QTableAgent::QTableAgent;

 public:
  ~QLearningAgent() override = default;

  static QLearningAgent& get() {
    if (Path.empty()) {
      throw std::logic_error{"agent path error"};
    }
    static QLearningAgent agent(Path, Logger);
    return agent;
  }

  static void setPath(const std::string& path) { Path = path; }
  static void setLogger(const SuffixLogger& logger) { Logger = logger; }

  double updateQValues(const recorder::Actions& state,
                       const recorder::Actions& nextState,
                       const recorder::Action& action,
                       const double reward,
                       const double learningRate,
                       const double discount);

  static std::string Path;

  static SuffixLogger Logger;
};

}  // namespace qlearning
}  // namespace agent_model
}  // namespace cider
