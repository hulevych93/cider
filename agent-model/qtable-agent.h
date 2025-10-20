// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "agent-model/agent.h"

#include <unordered_map>

namespace cider {
namespace agent_model {

using QValues = std::unordered_map<recorder::Action,
                                   double,
                                   recorder::FuzzyActionHash,
                                   recorder::FuzzyEqualPred>;

using QTable = std::unordered_map<recorder::Actions,
                                  QValues,
                                  recorder::SemanticActionHash,
                                  recorder::SemanticEqualPred>;

struct QTableStats final {
  std::size_t numStates = 0;
  std::size_t totalActions = 0;
  double avgActionsPerState = 0.0;
  std::size_t maxActionsInState = 0;
  std::size_t minActionsInState = 0;
  std::size_t zeroActionStates = 0;
};

using SuffixLogger = std::function<void(int)>;

class QTableAgent : public IAgent {
 protected:
  explicit QTableAgent(const std::string& path, SuffixLogger suffixLogger = {});

 public:
  bool isLoaded() const override { return m_loaded; }

  bool load(const std::string& filePath) override;
  bool save() const override;

  std::optional<recorder::Action> chooseBoltzmannAction(
      const Scenario& scenario,
      const double temperature) const override;
  std::optional<recorder::Action> chooseBoltzmannWithOpenersAction(
      const Scenario& scenario,
      const double temperature,
      const double lambda,
      size_t top_k) const override;
  std::optional<recorder::Action> chooseEGreedyAction(
      const Scenario& scenario,
      const double exploration) const override;
  std::optional<recorder::Action> chooseGreedyAction(
      const Scenario& scenario) const override;

  void print(std::ostream& os) const override;
  void printMetrics(std::ostream& os, const Episode& episode) const override;

  size_t getMaxStateDepth() const override { return m_maxStateDepth; }

 protected:
  SuffixLogger m_suffixLogger;

  size_t m_maxStateDepth = 0U;
  QTable m_qtable;

  std::mt19937& _gen;
  bool m_loaded = false;
  std::string m_path;
};

}  // namespace agent_model
}  // namespace cider
