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

class QAgent {
 public:
  virtual ~QAgent() = default;

  virtual std::optional<QAction> chooseBolzmanAction(
      const Scenario& scenario,
      const double temperature) const = 0;
  virtual std::optional<QAction> chooseEGreedyAction(
      const Scenario& scenario,
      const double exploration) const = 0;
  virtual std::optional<QAction> chooseGreedyAction(
      const Scenario& scenario) const = 0;

  virtual double updateQValues(const std::string& state,
                               const std::string& nextState,
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
