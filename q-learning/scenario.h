// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "agent.h"

namespace cider {
namespace qleaning {

std::string actionToGenericRepro(const QAction& action);
std::string actionsToGenericRepro(const QActionList& actions);

struct RewardCounter final {
  int covGrow = 0;
  int trackGrow = 0;
  int covNearBest = 0;
  int twoSameAct = 0;
  int threeSameAct = 0;
  int sameCov = 0;
  int penalty = 0;
  int scriptLower = 0;
  int scriptSame = 0;
  int scriptBigger = 0;
};

void print(std::ostream& os, const RewardCounter& rwCounter);

class IScenario {
 public:
  virtual ~IScenario() = default;

  virtual void add(const QAction& action) = 0;
  virtual void rollback() = 0;

  virtual size_t getSize() const = 0;

  virtual QActionList getCurrentState() const = 0;
  virtual QActionList getResult() const = 0;

  virtual std::optional<QValue> getReward() const = 0;

  virtual QActionList getAvailableActions() const = 0;

  virtual std::optional<QAction> getRandomAction() const = 0;

  virtual bool isOver() const = 0;
};

}  // namespace qleaning
}  // namespace cider
