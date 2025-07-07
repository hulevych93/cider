// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "coverage/coverage.h"

#include "scenario.h"

#include <random>
#include <sstream>

namespace cider {
namespace qleaning {

class QAgent {
 public:
  virtual ~QAgent() = default;

  virtual std::optional<recorder::Action> chooseBolzmanAction(
      const Scenario& scenario,
      const double temperature) const = 0;

  virtual std::optional<recorder::Action> chooseEGreedyAction(
      const Scenario& scenario,
      const double exploration) const = 0;

  virtual std::optional<recorder::Action> chooseGreedyAction(
      const Scenario& scenario) const = 0;

  virtual double updateQValues(const recorder::Actions& state,
                               const recorder::Actions& nextState,
                               const recorder::Action& action,
                               const double reward,
                               const double learningRate,
                               const double discount) = 0;

  virtual void print(std::ostream& os) const = 0;

  virtual std::mt19937& getSeed() = 0;

  virtual bool isLoaded() const = 0;

  virtual bool load(const std::string& filePath) = 0;
  virtual bool save(const std::string& filePath) const = 0;
};

QAgent& getAgent(const std::string& path = "");

}  // namespace qleaning
}  // namespace cider
