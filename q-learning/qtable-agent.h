// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "q-learning/agent.h"

#include <unordered_map>

namespace cider {
namespace qleaning {

using QValues = std::unordered_map<QAction,
                                   QValue,
                                   recorder::FuzzyActionHash,
                                   recorder::FuzzyEqualPred>;

using QTable = std::unordered_map<QActionList,
                                  QValues,
                                  recorder::SemanticActionHash,
                                  recorder::SemanticEqualPred>;

class QTableAgent final : public QAgent {
  static QActionList getBestFromAvailable(const QActionList& available,
                                          const QValues& values);

  std::optional<QAction> findBestOrRandomAvailableAction(
      const IScenario& scenario) const;

  explicit QTableAgent(const std::string& path);

  friend QAgent& getAgent(const std::string& path);

 public:
  bool isLoaded() const override { return m_loaded; }

  bool load(const std::string& filePath) override;
  bool save(const std::string& filePath) const override;

  std::optional<QAction> chooseBolzmanAction(
      const IScenario& scenario,
      const double temperature) const override;
  std::optional<QAction> chooseEGreedyAction(
      const IScenario& scenario,
      const double exploration) const override;
  std::optional<QAction> chooseGreedyAction(
      const IScenario& scenario) const override;
  std::optional<QAction> chooseRandAction(
      const IScenario& scenario) const override;

  double updateQValues(const QActionList& state,
                       const QActionList& nextState,
                       const QAction& action,
                       const QValue reward,
                       const double learningRate,
                       const double discount) override;

  void print(std::ostream& os) const override;

  std::mt19937& getSeed() override { return _gen; }

 private:
  QTable m_qtable;

  std::random_device rd;
  mutable std::mt19937 _gen;
  bool m_loaded = false;
};

}  // namespace qleaning
}  // namespace cider
