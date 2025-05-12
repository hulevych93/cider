// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "agent.h"

#include "coverage/coverage.h"

namespace cider {
namespace qleaning {

std::string actionToGenericRepro(const QAction& action);

using ObjectiveFunction =
    std::function<ObjectiveValue(const std::vector<QAction>&)>;

class Scenario final {
 public:
  Scenario(std::mt19937& gen,
           int maxStateDepth,
           const QActionList& initial,
           const ObjectiveFunction& objFunc);

  void add(const QAction& action);
  void rollback();
  void rollbackAndDrop();
  void drop(const QAction& action);

  QActionList getCurrentState() const;

  std::optional<QValue> getReward() const;

  std::string toString() const;

  QActionList getAvailableActions() const;

  std::optional<QAction> getRandomAction() const;

  bool isOver() const;

 private:
  std::mt19937& m_gen;

  int m_maxStateDepth = 0;

  QActionList m_actions;
  mutable ObjectiveValue m_lastObjVal;

  QActionSet m_availableActions;
  const int m_size;
  ObjectiveValue m_initialObjVal;

  ObjectiveFunction m_objFunc;
};

}  // namespace qleaning
}  // namespace cider
