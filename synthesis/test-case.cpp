// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "test-case.h"

#include <iostream>
#include <thread>

#include <assert.h>
#include <random>

namespace cider {
namespace synthesis {

bool isOverFunc(const ObjectiveValue& objValue,
                const ObjectiveValue& targetValue,
                const bool noActions) {
  const auto coverageBigger = objValue.coverage > targetValue.coverage;
  const auto coverageSame = abs(objValue.coverage - targetValue.coverage) <
                            std::numeric_limits<double>::epsilon();
  return coverageBigger || coverageSame || noActions;
}

TestScenario::TestScenario(std::mt19937& gen,
                           const recorder::Actions& initial,
                           const ObjectiveFunction& objFunc)
    : _gen(gen), _initialSize(initial.size()), _objFunc(objFunc) {
  for (const auto& action : initial) {
    _availableActions.emplace(action);
  }

  const auto objValue = _objFunc(initial);
  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
    std::cout << "Initial: " << objValue.coverage << std::endl;
    _initialObjVal = objValue;
  } else {
    throw std::logic_error{"Bad initial script."};
  }
}

void TestScenario::add(const recorder::Action& action) {
  _actions.push_back(action);

  const auto avIt = _availableActions.find(action);
  if (avIt != _availableActions.cend()) {
    _availableActions.erase(avIt);
  }
}

void TestScenario::rollback() {
  const auto& action = _actions.back();
  _availableActions.emplace(action);

  _actions.pop_back();
}

recorder::Actions TestScenario::getResult() const {
  return _actions;
}

bool TestScenario::isValid() const {
  const auto objValue = _objFunc(_actions);
  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
    std::cout << "Old: " << _initialSize << ", new: " << _actions.size()
              << std::endl;

    _lastObjVal = objValue;
    return true;
  }
  return false;
}

bool TestScenario::isOver() const {
  return isOverFunc(_lastObjVal, _initialObjVal, _availableActions.empty());
}

recorder::Actions TestScenario::getAvailableActions() const {
  recorder::Actions actions;
  actions.reserve(_availableActions.size());
  for (const auto& action : _availableActions) {
    actions.emplace_back(action);
  }
  return actions;
}

std::optional<recorder::Action> TestScenario::getRandomAction() const {
  const auto actions = getAvailableActions();
  if (actions.empty()) {
    return std::nullopt;
  }
  std::uniform_int_distribution<size_t> indexDist(0, actions.size() - 1);
  return actions[indexDist(_gen)];
}

}  // namespace synthesis
}  // namespace cider
