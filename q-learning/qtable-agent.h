// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "q-learning/agent.h"

#include <unordered_map>

namespace cider {
namespace qleaning {

using QValues = std::unordered_map<recorder::Action,
                                   double,
                                   recorder::FuzzyActionHash,
                                   recorder::FuzzyEqualPred>;

using QTable = std::unordered_map<recorder::Actions,
                                  QValues,
                                  recorder::SemanticActionHash,
                                  recorder::SemanticEqualPred>;

class QTableAgent final : public QAgent {
  static recorder::Actions getBestFromAvailable(
      const recorder::Actions& available,
      const QValues& values);

  std::optional<recorder::Action> findBestOrRandomAvailableAction(
      const Scenario& scenario) const;

  explicit QTableAgent(const std::string& path);

  friend QAgent& getAgent(const std::string& path);

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

  double updateQValues(const recorder::Actions& state,
                       const recorder::Actions& nextState,
                       const recorder::Action& action,
                       const double reward,
                       const double learningRate,
                       const double discount) override;

  void print(std::ostream& os) const override;

  std::mt19937& getSeed() override { return _gen; }

 private:
  QTable m_qtable;

  std::mt19937& _gen;
  bool m_loaded = false;
};

}  // namespace qleaning
}  // namespace cider
