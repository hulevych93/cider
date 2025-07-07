// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "synthesis/test-case.h"

namespace cider {
namespace qleaning {

class Scenario : public synthesis::TestScenario {
 public:
  Scenario(std::mt19937& gen,
           int maxStateDepth,
           const recorder::Actions& initial,
           const synthesis::ObjectiveFunction& objFunc);

  recorder::Actions getCurrentState() const;

 private:
  int _maxStateDepth = 0;
};

std::string actionToGenericRepro(const recorder::Action& action);
std::string actionsToGenericRepro(const std::vector<recorder::Action>& actions);

}  // namespace qleaning
}  // namespace cider
