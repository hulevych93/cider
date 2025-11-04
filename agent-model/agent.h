// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "coverage/coverage.h"

#include "agent-model/scenario.h"

#include <random>
#include <sstream>

namespace cider {
namespace agent_model {

struct Episode final {
  int episode = 0;
  std::chrono::system_clock::time_point start;
  std::chrono::system_clock::time_point end;
};

class IAgent {
 public:
  virtual ~IAgent() = default;

  virtual std::optional<recorder::Action> chooseBoltzmannAction(
      const Scenario& scenario,
      const double temperature,
      const double lambda) const = 0;

  virtual std::optional<recorder::Action> chooseBoltzmannWithOpenersAction(
      const Scenario& scenario,
      const double temperature,
      const double lambda,
      size_t top_k) const = 0;

  virtual std::optional<recorder::Action> chooseEGreedyAction(
      const Scenario& scenario,
      const double exploration) const = 0;

  virtual std::optional<recorder::Action> chooseGreedyAction(
      const Scenario& scenario) const = 0;

  virtual void print(std::ostream& os) const = 0;
  virtual void printMetrics(std::ostream& os, const Episode& episode) const = 0;

  virtual bool isLoaded() const = 0;

  virtual bool load(const std::string& filePath) = 0;
  virtual bool save() const = 0;

  virtual size_t getMaxStateDepth() const = 0;
};

}  // namespace agent_model
}  // namespace cider
