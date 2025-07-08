// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"

#include <random>

namespace cider {
namespace synthesis {

using ActionSet = std::unordered_set<recorder::Action,
                                     recorder::FuzzyActionHash,
                                     recorder::FuzzyEqualPred>;

using ObjectiveFunction =
    std::function<ObjectiveValue(const recorder::Actions&)>;

class TestScenario {
 public:
  TestScenario(std::mt19937& gen,
               const recorder::Actions& initial,
               const ObjectiveFunction& objFunc);
  virtual ~TestScenario() = default;

  void add(const recorder::Action& action);
  void rollback();

  size_t getSize() const { return _actions.size(); }

  recorder::Actions getResult() const;
  recorder::Actions getAvailableActions() const;

  std::optional<recorder::Action> getRandomAction() const;

  bool isValid(bool storeCoverage = true) const;
  bool isOver() const;

 protected:
  std::mt19937& _gen;

  mutable recorder::Actions _actions;

  ActionSet _availableActions;
  const int _initialSize;

  ObjectiveValue _initialObjVal;
  mutable ObjectiveValue _lastObjVal;

  ObjectiveFunction _objFunc;
};

bool isOverFunc(const ObjectiveValue& objValue,
                const ObjectiveValue& targetValue,
                const bool noActions);

}  // namespace synthesis
}  // namespace cider
