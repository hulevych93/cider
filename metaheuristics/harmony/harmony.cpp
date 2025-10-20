// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "harmony.h"

#include "coverage/coverage.h"
#include "metaheuristics/args_mutator.h"

#include <assert.h>
#include <tlog.h>

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
    : _gen(Seed::instance().get()),
      _settings(settings),
      _mutator(cider::metasearch::makeMutator(_gen,
                                              settings.mutationRate,
                                              settings.strategy)) {}

void Search::initialize(const ActionsCallback& callback) {
  _actionsGenerator = callback;

  _harmonyMemory.resize(_settings.harmonyMemorySize);
  for (auto i = 0U; i < _settings.harmonyMemorySize; ++i) {
    auto actions = _actionsGenerator();
    const auto objValue = _settings.objFunc(actions).coverage;
    if (objValue > std::numeric_limits<double>::epsilon()) {
      Solution solution;
      solution.actions = std::move(actions);
      solution.objVal = objValue;
      _harmonyMemory[i] = std::move(solution);
    } else {
      tlog_info << "Bad script during nest generation" << std::endl;
    }
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
    if (_settings.maxIter != 0 && iteration > _settings.maxIter) {
      break;
    }

    tlog_info << "Iter: " << iteration << std::endl;
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
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  if (dist(_gen) < _settings.harmonyMemoryConsiderationRate) {
    newHarmony = deepCopy(harmony);
  } else {
    auto actions = _actionsGenerator();
    const auto objValue = _settings.objFunc(actions).coverage;
    if (objValue > std::numeric_limits<double>::epsilon()) {
      newHarmony.actions = std::move(actions);
      newHarmony.objVal = objValue;
    } else {
      tlog_info << "Bad script during nest generation" << std::endl;
    }
  }
  return newHarmony;
}

std::optional<Harmony> Search::mutateHarmony(const Harmony& harmony) const {
  Harmony mutatedHarmony = deepCopy(harmony);

  bool isMutated = false;

  if (_settings.instructionsMutationStrategy ==
      InstructionsMutationStrategy::Shuffle) {
    std::shuffle(mutatedHarmony.actions.begin(), mutatedHarmony.actions.end(),
                 _gen);
    isMutated = true;
  }

  for (auto& action : mutatedHarmony.actions) {
    recorder::ActionMutator mutator(*_mutator);
    isMutated |= std::visit(mutator, action);
  }

  if (!isMutated) {
    return std::nullopt;
  }

  const auto objValue = _settings.objFunc(mutatedHarmony.actions).coverage;
  if (objValue > std::numeric_limits<double>::epsilon()) {
    mutatedHarmony.objVal = objValue;
    tlog_info << objValue << std::endl;
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
  tlog_info << "Best :" << best.objVal << std::endl;
  return best;
}

void Search::dump() {
  int idx = 0;
  tlog_info << _harmonyMemory.size() << std::endl;
  for (const auto& harmony : _harmonyMemory) {
    tlog_info << "Harmony[" << idx << "]: " << harmony.objVal << std::endl;
    ++idx;
  }
}

}  // namespace harmony
}  // namespace metasearch
}  // namespace cider
