// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "harmony_synthesis.h"

#include "coverage/coverage.h"
#include "metaheuristics/args_mutator.h"

#include "synthesis/synthesis.h"

#include <assert.h>
#include <tlog.h>

namespace cider {
namespace metasearch {
namespace harmony_synthesis {

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

Search::Search(const Settings& settings) : harmony::Search(settings) {}

harmony::Harmony Search::generateHarmony(
    const harmony::Harmony& harmony) const {
  harmony::Harmony newHarmony;
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  if (dist(_gen) < _settings.harmonyMemoryConsiderationRate) {
    synthesis::TestScenario scenario(
        _gen, harmony.actions, [&](const auto& actions) {
          ObjectiveValue val;
          val.coverage = _settings.objFunc(actions);
          return val;
        });

    const auto actionChoosing = [&](const synthesis::TestScenario& testCase) {
      return testCase.getRandomAction();
    };

    synthesis::SynthesisSettingsBasic settings;
    settings.configName = "GRAND";
    settings.stopType = synthesis::StopCondition::GreaterCoverage;

    synthesis::details::synthesize(settings, actionChoosing, scenario);

    newHarmony.actions = scenario.getResult();

  } else {
    newHarmony.actions = _actionsGenerator();
  }

  const auto objValue = _settings.objFunc(newHarmony.actions);
  if (objValue > std::numeric_limits<double>::epsilon()) {
    newHarmony.objVal = objValue;
  } else {
    tlog_info << "Bad script during nest generation" << std::endl;
  }

  return newHarmony;
}

std::optional<harmony::Harmony> Search::mutateHarmony(
    const harmony::Harmony& harmony) const {
  harmony::Harmony mutatedHarmony = deepCopy(harmony);

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

  const auto objValue = _settings.objFunc(mutatedHarmony.actions);
  if (objValue > std::numeric_limits<double>::epsilon()) {
    mutatedHarmony.objVal = objValue;
    tlog_info << objValue << std::endl;
    return mutatedHarmony;
  }

  return std::nullopt;
}

}  // namespace harmony_synthesis
}  // namespace metasearch
}  // namespace cider
