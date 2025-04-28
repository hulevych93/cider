// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include <unordered_set>

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

class Agent {
 public:
  virtual ~Agent() = default;

  virtual QAction chooseAction(const Scenario& scenario) const = 0;
};

class QValuesAgent final : public Agent {
  static QActionList getBestFromAvailable(const QActionList& available,
                                          const QValues& values);

  QAction findBestOrRandomAvailableAction(const Scenario& scenario) const;

 public:
  QAction chooseAction(const Scenario& scenario, const double exploration);

  QAction chooseAction(const Scenario& scenario) const override;

  void updateQValues(const std::string& state,
                     const std::string& nextState,
                     const QAction& action,
                     const double reward,
                     const double learningRate,
                     const double discount);

  void print(std::ostream& os) const;

 private:
  QTable m_qtable;
};

class RandomAgent final : public Agent {
 public:
  QAction chooseAction(const Scenario& scenario) const override;
};

class TeacherAgent final : public Agent {
 public:
  TeacherAgent();

  QAction chooseAction(const Scenario& scenario) const override {
    return m_list[0];  // TODO
  }

 private:
  QActionList m_list;
};

}  // namespace qleaning
}  // namespace cider
