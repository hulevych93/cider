// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "coverage/coverage.h"

#include <unordered_set>

#include <random>
#include <sstream>

namespace cider {
namespace qleaning {

class IScenario;
using QValue = double;
using Script = std::string;
using QAction = recorder::Action;
using QActionList = std::vector<QAction>;
using QActionSet = std::
    unordered_set<QAction, recorder::FuzzyActionHash, recorder::FuzzyEqualPred>;

using ObjectiveFunction =
    std::function<ObjectiveValue(const std::vector<QAction>&)>;

class QAgent {
 public:
  virtual ~QAgent() = default;

  virtual std::optional<QAction> chooseBolzmanAction(
      const IScenario& scenario,
      const double temperature) const = 0;
  virtual std::optional<QAction> chooseEGreedyAction(
      const IScenario& scenario,
      const double exploration) const = 0;
  virtual std::optional<QAction> chooseGreedyAction(
      const IScenario& scenario) const = 0;
  virtual std::optional<QAction> chooseRandAction(
      const IScenario& scenario) const = 0;

  virtual double updateQValues(const QActionList& state,
                               const QActionList& nextState,
                               const QAction& action,
                               const double reward,
                               const double learningRate,
                               const double discount) = 0;

  virtual void print(std::ostream& os) const = 0;

  virtual std::mt19937& getSeed() = 0;

  virtual bool isLoaded() const = 0;

  virtual bool load(const std::string& filePath) = 0;
  virtual bool save(const std::string& filePath) const = 0;
};

}  // namespace qleaning
}  // namespace cider
