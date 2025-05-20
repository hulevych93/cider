// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "agent.h"
#include "scenario.h"

#include "coverage/coverage.h"

namespace cider {
namespace qleaning {

class QScenario final : public IScenario {
 public:
  QScenario(RewardCounter& counter,
            std::mt19937& gen,
            int maxStateDepth,
            const QActionList& initial,
            const ObjectiveFunction& objFunc);

  void add(const QAction& action) override;
  void rollback() override;

  size_t getSize() const override { return m_actions.size(); }

  QActionList getCurrentState() const override;
  QActionList getResult() const override;

  std::optional<QValue> getReward() const override;

  QActionList getAvailableActions() const override;

  std::optional<QAction> getRandomAction() const override;

  bool isOver() const override;

  void store();

 private:
  std::mt19937& m_gen;

  int m_maxStateDepth = 0;

  mutable QActionList m_actions;
  mutable ObjectiveValue m_lastObjVal;

  mutable QActionList m_storage;

  QActionSet m_availableActions;
  const int m_initialSize;
  ObjectiveValue m_initialObjVal;

  ObjectiveFunction m_objFunc;

  RewardCounter& m_rwCounter;
};

}  // namespace qleaning
}  // namespace cider
