// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "metaheuristics/args_mutator.h"
#include "metaheuristics/metasearch.h"

namespace cider {
namespace metasearch {
namespace harmony {

struct Settings final {
  int harmonyMemorySize = 10;
  double harmonyMemoryConsiderationRate = 0.95;
  double mutationRate = 0.1;
  size_t maxIterationsWithoutUpdates = 500U;
  ObjectiveFunction objFunc;
  MutationStrategy strategy = MutationStrategy::ShuffleBytes;
};

std::ostream& operator<<(std::ostream& os, const Settings& settings);

using Harmony = Solution;

class Search final : public IMetaSearch {
 public:
  explicit Search(const Settings& settings);

  void initialize(const std::vector<recorder::Action>& actions) override;

  void run() override;

  const Harmony& getBest() const override;

  void setLogger(std::unique_ptr<IResultsLogger> logger) override {
    _logger = std::move(logger);
  }

 private:
  Harmony generateHarmony(const Harmony& harmony) const;
  std::optional<Harmony> mutateHarmony(const Harmony& harmony) const;
  bool updateHarmonyMemory(const Harmony& harmony);

  Harmony& getWorst();

  void dump();

 private:
  std::random_device _rd;
  mutable std::mt19937 _gen;

  Settings _settings;
  std::vector<Harmony> _harmonyMemory;
  Harmony _initial;
  std::unique_ptr<recorder::IParamMutator> _mutator;

  std::unique_ptr<IResultsLogger> _logger;
};

}  // namespace harmony
}  // namespace metasearch
}  // namespace cider
