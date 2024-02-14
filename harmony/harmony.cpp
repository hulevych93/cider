// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "harmony.h"

#include <assert.h>
#include <iostream>

namespace cider {
namespace harmony {

bool operator>(const coverage::CoverageReport& left,
               const coverage::CoverageReport& right) {
  return left.lineCov.covered > right.lineCov.covered;
}

bool operator>(const coverage::RootReport& left,
               const coverage::RootReport& right) {
  return left.report > right.report;
}

bool operator<(const coverage::CoverageReport& left,
               const coverage::CoverageReport& right) {
  return left.lineCov.covered < right.lineCov.covered;
}

bool operator<(const coverage::RootReport& left,
               const coverage::RootReport& right) {
  return left.report < right.report;
}

struct ActionMutator final {
  explicit ActionMutator(const recorder::IParamMutator& mutator)
      : _mutator(mutator) {}

  void operator()(recorder::Function& context) {
    for (auto& param : context.params) {
      std::visit(_mutator, param);
    }
  }

  void operator()(recorder::ClassMethod& context) {
    for (auto& param : context.method.params) {
      std::visit(_mutator, param);
    }
  }

  void operator()(recorder::ClassBinaryOp& context) {
    std::visit(_mutator, context.param);
  }

  void operator()(recorder::ClassUnaryOp&) {}
  void operator()(recorder::ClassDestructor&) {}

  const recorder::IParamMutator& _mutator;
};

Search::Search(const Settings& settings)
    : _settings(settings),
      _mutator(makeMutator(settings.mutationRate, settings.strategy)) {}

void Search::initialize(const std::vector<recorder::Action>& actions) {
  _initial.cov = _settings.meassure(actions).value();
  _initial.actions = actions;

  _harmonyMemory.resize(_settings.harmonyMemorySize);
  for (auto i = 0U; i < _settings.harmonyMemorySize; ++i) {
    _harmonyMemory[i] = _initial;
  }
}

void Search::run() {
  for (int iteration = 0; iteration < _settings.maxIterations; ++iteration) {
    std::cout << "Iter: " << iteration << std::endl;
    Harmony newHarmony =
        generateHarmony(_harmonyMemory[rand() % _settings.harmonyMemorySize]);
    newHarmony = mutateHarmony(newHarmony);
    updateHarmonyMemory(newHarmony);
  }
}

Harmony Search::generateHarmony(const Harmony& harmony) const {
  Harmony newHarmony;
  if (rand() / static_cast<double>(RAND_MAX) <
      _settings.harmonyMemoryConsiderationRate) {
    newHarmony = harmony;
  } else {
    newHarmony = _initial;
  }
  return harmony;
}

Harmony Search::mutateHarmony(const Harmony& harmony) const {
  Harmony mutatedHarmony = harmony;

  for (auto& action : mutatedHarmony.actions) {
    ActionMutator mutator(*_mutator);
    std::visit(mutator, action);
  }

  if (const auto& covOpt = _settings.meassure(mutatedHarmony.actions)) {
    mutatedHarmony.cov = covOpt.value();
    return mutatedHarmony;
  }
  return harmony;
}

void Search::updateHarmonyMemory(const Harmony& harmony) {
  auto worstHarmony = std::min_element(
      _harmonyMemory.begin(), _harmonyMemory.end(),
      [](const Harmony& h1, const Harmony& h2) { return h1.cov < h2.cov; });
  if (harmony.cov > worstHarmony->cov) {
    *worstHarmony = harmony;
    dump();
  }
}

const Harmony& Search::getBest() const {
  return *std::max_element(
      _harmonyMemory.begin(), _harmonyMemory.end(),
      [](const Harmony& h1, const Harmony& h2) { return h1.cov > h2.cov; });
}

void printTableEntry(std::ostream& ss,
                     const cider::coverage::CoverageReport& report) {
  ss << report.lineCov.percent << "\t" << report.branchCov.percent << "\t"
     << report.funcCov.percent << std::endl;
}

void Search::dump() {
  for (const auto& harmony : _harmonyMemory) {
    printTableEntry(std::cout, harmony.cov.report);
  }
}

}  // namespace harmony
}  // namespace cider
