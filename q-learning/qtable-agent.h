// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "q-learning/agent.h"

#include <unordered_map>

namespace cider {
namespace qleaning {

struct FuzzyEqualPred {
  bool operator()(const QAction& lhs, const QAction& rhs) const {
    return recorder::fuzzyEqual(lhs, rhs);
  }
};

using QValues =
    std::unordered_map<QAction, QValue, std::hash<QAction>, FuzzyEqualPred>;
using QTable = std::unordered_map<Script, QValues>;

class QTableAgent final : public QAgent {
  static QActionList getBestFromAvailable(const QActionList& available,
                                          const QValues& values);

  std::optional<QAction> findBestOrRandomAvailableAction(
      const Scenario& scenario) const;

  explicit QTableAgent(const std::string& path);

  friend QAgent& getAgent(const std::string& path);

 public:
  bool isLoaded() const override { return m_loaded; }

  bool load(const std::string& filePath) override;
  bool save(const std::string& filePath) const override;

  std::optional<QAction> chooseBolzmanAction(
      const Scenario& scenario,
      const double temperature) const override;
  std::optional<QAction> chooseEGreedyAction(
      const Scenario& scenario,
      const double exploration) const override;
  std::optional<QAction> chooseGreedyAction(
      const Scenario& scenario) const override;
  std::optional<QAction> chooseRandAction(
      const Scenario& scenario) const override;

  double updateQValues(const std::string& state,
                       const std::string& nextState,
                       const QAction& action,
                       const double reward,
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
