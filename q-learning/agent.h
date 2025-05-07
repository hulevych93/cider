// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include <unordered_set>

#include <random>
#include <sstream>

namespace cider {
namespace qleaning {

class Scenario;

using QValue = double;
using Script = std::string;

using QAction = recorder::Action;
using QActionList = std::vector<QAction>;
using QActionSet = std::unordered_set<QAction>;
using QValues = std::unordered_map<QAction, QValue>;
using QTable = std::unordered_map<Script, QValues>;

class QValuesAgent final {
  static QActionList getBestFromAvailable(const QActionList& available,
                                          const QValues& values);

  std::optional<QAction> findBestOrRandomAvailableAction(
      const Scenario& scenario) const;

 public:
  explicit QValuesAgent();

  bool load(const std::string& filePath);
  bool save(const std::string& filePath);

  std::optional<QAction> chooseBolzmanAction(const Scenario& scenario,
                                             const double temperature) const;
  std::optional<QAction> chooseEGreedyAction(const Scenario& scenario,
                                             const double exploration) const;
  std::optional<QAction> chooseGreedyAction(const Scenario& scenario) const;

  void updateQValues(const std::string& state,
                     const std::string& nextState,
                     const QAction& action,
                     const double reward,
                     const double learningRate,
                     const double discount);

  QValues getQValues(const std::string& state);

  void print(std::ostream& os) const;

  std::mt19937& getSeed() { return _gen; }

 private:
  QTable m_qtable;

  std::random_device rd;
  mutable std::mt19937 _gen;
};

}  // namespace qleaning
}  // namespace cider
