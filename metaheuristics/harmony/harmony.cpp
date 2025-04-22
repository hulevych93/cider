// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "harmony.h"

#include "metaheuristics/args_mutator.h"

#include <assert.h>
#include <iostream>

namespace cider {
namespace metasearch {
namespace harmony {

namespace {

struct ActionMutator final {
  explicit ActionMutator(const recorder::IParamMutator& mutator)
      : _mutator(mutator) {}

  bool operator()(recorder::Function& context) {
    bool isMutated = false;
    for (auto& param : context.params) {
      isMutated |= std::visit(_mutator, param);
    }
    return isMutated;
  }

  bool operator()(recorder::ClassMethod& context) {
    bool isMutated = false;
    for (auto& param : context.method.params) {
      isMutated |= std::visit(_mutator, param);
    }
    return isMutated;
  }

  bool operator()(recorder::ClassBinaryOp& context) {
    return std::visit(_mutator, context.param);
  }

  bool operator()(recorder::ClassUnaryOp&) { return false; }
  bool operator()(recorder::ClassDestructor&) { return false; }

  const recorder::IParamMutator& _mutator;
};

}  // namespace

Search::Search(const Settings& settings)
    : _gen(_rd()),
      _settings(settings),
      _mutator(cider::metasearch::makeMutator(_gen,
                                              settings.mutationRate,
                                              settings.strategy)) {}

void Search::initialize(const std::vector<recorder::Action>& actions) {
  _initial.actions = actions;

  if (const auto& covOpt = _settings.meassure(_initial.actions)) {
    _initial.cov = covOpt.value();
  } else {
    throw std::logic_error{"Bad initial script."};
  }

  _harmonyMemory.resize(_settings.harmonyMemorySize);
  for (auto i = 0U; i < _settings.harmonyMemorySize; ++i) {
    _harmonyMemory[i] = _initial;
  }

  dump();
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
  std::uniform_int_distribution<> distr(0, 10000);
  if (((double)distr(_gen) / 10000.f) <
      _settings.harmonyMemoryConsiderationRate) {
    newHarmony = harmony;
  } else {
    newHarmony = _initial;
  }
  return newHarmony;
}

std::optional<Harmony> Search::mutateHarmony(const Harmony& harmony) const {
  Harmony mutatedHarmony = harmony;

  bool isMutated = false;
  for (auto& action : mutatedHarmony.actions) {
    ActionMutator mutator(*_mutator);
    isMutated |= std::visit(mutator, action);
  }

  if (!isMutated) {
    return std::nullopt;
  }

  if (const auto& covOpt = _settings.meassure(mutatedHarmony.actions)) {
    mutatedHarmony.cov = covOpt.value();
    std::cout << "Candidate: ";
    cider::coverage::printTableEntry(std::cout, 0, mutatedHarmony.cov.report);
    return mutatedHarmony;
  }
  return std::nullopt;
}

bool Search::updateHarmonyMemory(const Harmony& harmony) {
  auto& worstHarmony = getWorst();
  if (harmony.cov > worstHarmony.cov) {
    worstHarmony = harmony;
    dump();
    return true;
  }
  return false;
}

Harmony& Search::getWorst() {
  auto worstHarmony =
      std::min_element(_harmonyMemory.begin(), _harmonyMemory.end());
  return *worstHarmony;
}

const Harmony& Search::getBest() const {
  const auto& best =
      *std::max_element(_harmonyMemory.begin(), _harmonyMemory.end());
  std::cout << "Best :";
  cider::coverage::printTableEntry(std::cout, 0, best.cov.report);
  return best;
}

void Search::dump() {
  for (const auto& harmony : _harmonyMemory) {
    cider::coverage::printTableEntry(std::cout, 0, harmony.cov.report);
  }
}

}  // namespace harmony
}  // namespace metasearch
}  // namespace cider
