// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "coverage/coverage.h"
#include "coverage/measurer.h"

#include <unordered_set>

#include "agent.h"

namespace cider {
namespace qleaning {

std::string actionToFullString(const QAction& action);
std::string actionToShortString(const QAction& action);

using MeassureCallback =
    std::function<cider::coverage::ReportOpt(const std::vector<QAction>&)>;

class Scenario final {
 public:
  Scenario(const QActionList& initial,
           const MeassureCallback& meassurer);

  void add(const QAction& action);
  void rollback();

  std::optional<QValue> getReward() const;

  std::string toString() const;

  QActionList getAvailableActions() const;

  QAction getRandomAction() const;

  bool isOver() const;

 private:
  QActionList m_actions;
  mutable coverage::RootReport m_lastCov;

  QActionSet m_availableActions;
  const int m_size;
  coverage::RootReport m_initialCov;

  MeassureCallback m_meassurer;
};

}  // namespace qleaning
}  // namespace cider
