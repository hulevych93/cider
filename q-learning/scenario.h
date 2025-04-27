// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include <unordered_set>

#include "agent.h"

namespace cider {
namespace qleaning {

std::string actionToFullString(const QAction& action);
std::string actionToShortString(const QAction& action);

using ObjectiveFunction = std::function<double(const std::vector<QAction>&)>;

class Scenario final {
 public:
  Scenario(const QActionList& initial, const ObjectiveFunction& objFunc);

  void add(const QAction& action);
  void rollback();

  std::optional<QValue> getReward() const;

  std::string toString() const;

  QActionList getAvailableActions() const;

  QAction getRandomAction() const;

  bool isOver() const;

 private:
  QActionList m_actions;
  mutable double m_lastObjVal = 0.0f;

  QActionSet m_availableActions;
  const int m_size;
  double m_initialObjVal = 0.0f;

  ObjectiveFunction m_objFunc;
};

}  // namespace qleaning
}  // namespace cider
