// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "harmony.h"

#include "metaheuristics/args_mutator.h"

#include <assert.h>
#include <iostream>

namespace cider {
namespace metasearch {
namespace harmony {

std::ostream& operator<<(std::ostream& os, const Settings& settings) {
  os << "HS_";
  os << "hms[";
  os << settings.harmonyMemorySize;
  os << "]_mr[";
  os << settings.mutationRate;
  os << "]_hmcs[";
  os << settings.harmonyMemoryConsiderationRate;
  os << "]_itwu[";
  os << settings.maxIterationsWithoutUpdates;
  os << "]_st[";
  os << settings.strategy;
  os << "]";
  return os;
}

Search::Search(const Settings& settings)
    : _gen(_rd()),
      _settings(settings),
      _mutator(cider::metasearch::makeMutator(_gen,
                                              settings.mutationRate,
                                              settings.strategy)) {}

void Search::initialize(const std::vector<recorder::Action>& actions) {
  _initial.actions = deepCopy(actions);

  const auto objValue = _settings.objFunc(_initial.actions);
  if (objValue > std::numeric_limits<double>::epsilon()) {
    _initial.objVal = objValue;
  } else {
    throw std::logic_error{"Bad initial script."};
  }

  _harmonyMemory.resize(_settings.harmonyMemorySize);
  for (auto i = 0U; i < _settings.harmonyMemorySize; ++i) {
    _harmonyMemory[i] = deepCopy(_initial);
  }

  if (_logger) {
    _logger->log(0U, getBest());
  }

  dump();
}

void Search::run() {
  size_t iterWithoutUpdates = 0U;
  for (int iteration = 1U;
       iterWithoutUpdates <= _settings.maxIterationsWithoutUpdates;
       ++iteration, ++iterWithoutUpdates) {
    std::cout << "Iter: " << iteration << std::endl;
    Harmony newHarmony = generateHarmony(getWorst());
    if (auto mutated = mutateHarmony(newHarmony)) {
      if (updateHarmonyMemory(*mutated)) {
        iterWithoutUpdates = 0U;

        if (_logger) {
          _logger->log(iteration, getBest());
        }
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
    newHarmony = deepCopy(harmony);
  } else {
    newHarmony = deepCopy(_initial);
  }
  return newHarmony;
}

std::optional<Harmony> Search::mutateHarmony(const Harmony& harmony) const {
  Harmony mutatedHarmony = deepCopy(harmony);

  bool isMutated = false;
  for (auto& action : mutatedHarmony.actions) {
    recorder::ActionMutator mutator(*_mutator);
    isMutated |= std::visit(mutator, action);
  }

  if (!isMutated) {
    return std::nullopt;
  }

  const auto objValue = _settings.objFunc(mutatedHarmony.actions);
  if (objValue > std::numeric_limits<double>::epsilon()) {
    mutatedHarmony.objVal = objValue;
    std::cout << objValue << std::endl;
    return mutatedHarmony;
  }

  return std::nullopt;
}

bool Search::updateHarmonyMemory(const Harmony& harmony) {
  auto& worstHarmony = getWorst();
  if (harmony.objVal > worstHarmony.objVal) {
    worstHarmony = deepCopy(harmony);
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
  std::cout << "Best :" << best.objVal << std::endl;
  return best;
}

void Search::dump() {
  int idx = 0;
  for (const auto& harmony : _harmonyMemory) {
    std::cout << "Harmony[" << idx << "]: " << harmony.objVal << std::endl;
    ++idx;
  }
}

}  // namespace harmony
}  // namespace metasearch
}  // namespace cider
