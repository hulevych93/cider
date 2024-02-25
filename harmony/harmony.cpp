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

bool operator>(const harmony::Harmony& left, const harmony::Harmony& right) {
  return left.cov > right.cov;
}

bool operator<(const coverage::CoverageReport& left,
               const coverage::CoverageReport& right) {
  return left.lineCov.covered < right.lineCov.covered;
}

bool operator<(const coverage::RootReport& left,
               const coverage::RootReport& right) {
  return left.report < right.report;
}

bool operator<(const harmony::Harmony& left, const harmony::Harmony& right) {
  return left.cov < right.cov;
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
  _initial.actions = actions;

  _harmonyMemory.resize(_settings.harmonyMemorySize);
  for (auto i = 0U; i < _settings.harmonyMemorySize; ++i) {
    _harmonyMemory[i] = _initial;
  }
}

void Search::run() {
  size_t iterWithoutUpdates = 0U;
  for (int iteration = 0;
       iterWithoutUpdates <= _settings.maxIterationsWithoutUpdates;
       ++iteration, ++iterWithoutUpdates) {
    std::cout << "Iter: " << iteration << std::endl;
    Harmony newHarmony = generateHarmony(getWorst());
    if (auto mutated = mutateHarmony(newHarmony)) {
      if (updateHarmonyMemory(*mutated)) {
        iterWithoutUpdates = 0U;
      }
    }
  }
  dump();
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

std::optional<Harmony> Search::mutateHarmony(const Harmony& harmony) const {
  Harmony mutatedHarmony = harmony;

  for (auto& action : mutatedHarmony.actions) {
    ActionMutator mutator(*_mutator);
    std::visit(mutator, action);
  }

  if (const auto& covOpt = _settings.meassure(mutatedHarmony.actions)) {
    mutatedHarmony.cov = covOpt.value();
    return mutatedHarmony;
  }
  return std::nullopt;
}

bool Search::updateHarmonyMemory(const Harmony& harmony) {
  auto& worstHarmony = getWorst();
  if (harmony.cov > worstHarmony.cov) {
    worstHarmony = harmony;
    dump();
    return false;
  }
  return false;
}

Harmony& Search::getWorst() {
  auto worstHarmony =
      std::min_element(_harmonyMemory.begin(), _harmonyMemory.end());
  return *worstHarmony;
}

void printTableEntry(std::ostream& ss,
                     const cider::coverage::CoverageReport& report) {
  ss << report.lineCov.percent << "\t" << report.branchCov.percent << "\t"
     << report.funcCov.percent << std::endl;
}

const Harmony& Search::getBest() const {
  const auto& best =
      *std::max_element(_harmonyMemory.begin(), _harmonyMemory.end());
  std::cout << "Best :";
  printTableEntry(std::cout, best.cov.report);
  return best;
}

void Search::dump() {
  for (const auto& harmony : _harmonyMemory) {
    printTableEntry(std::cout, harmony.cov.report);
  }
}

}  // namespace harmony
}  // namespace cider
