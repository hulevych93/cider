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

class QTableAgent : public IAgent {
  static recorder::Actions getBestFromAvailable(
      const recorder::Actions& available,
      const QValues& values);

  std::optional<recorder::Action> findBestOrRandomAvailableAction(
      const Scenario& scenario) const;

 protected:
  explicit QTableAgent(const std::string& path);

 public:
  bool isLoaded() const override { return m_loaded; }

  bool load(const std::string& filePath) override;
  bool save(const std::string& filePath) const override;

  std::optional<recorder::Action> chooseBolzmanAction(
      const Scenario& scenario,
      const double temperature) const override;
  std::optional<recorder::Action> chooseEGreedyAction(
      const Scenario& scenario,
      const double exploration) const override;
  std::optional<recorder::Action> chooseGreedyAction(
      const Scenario& scenario) const override;

  void print(std::ostream& os) const override;

  size_t getMaxStateDepth() const override { return m_maxStateDepth; }

 protected:
  size_t m_maxStateDepth = 0U;
  QTable m_qtable;

  std::mt19937& _gen;
  bool m_loaded = false;
};

}  // namespace agent_model
}  // namespace cider
