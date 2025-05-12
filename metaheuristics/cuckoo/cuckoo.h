// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "metaheuristics/args_mutator.h"
#include "metaheuristics/metasearch.h"

#include <random>

namespace cider {
namespace metasearch {
namespace cuckoo {

struct Settings final {
  int populationSize = 10;
  double Pa = 0.25;
  size_t maxIterationsWithoutUpdates = 50U;
  ObjectiveFunction objFunc;
  MutationStrategy strategy = MutationStrategy::ShuffleBytes;
};

std::ostream& operator<<(std::ostream& os, const Settings& settings);

using Nest = Solution;

class Search final : public IMetaSearch {
 public:
  explicit Search(const Settings& settings);

  void initialize(const std::vector<recorder::Action>& actions) override;

  void run() override;

  const Nest& getBest() const override;

  void setLogger(std::unique_ptr<IResultsLogger> logger) override {
    _logger = std::move(logger);
  }

 private:
  std::optional<Nest> generateNest(const Nest& nest) const;

  void dump();

 private:
  std::random_device _rd;
  mutable std::mt19937 _gen;

  Settings _settings;
  std::vector<Nest> _memory;
  Nest _initial;
  std::unique_ptr<recorder::IParamMutator> _mutator;

  std::unique_ptr<IResultsLogger> _logger;
};

}  // namespace cuckoo
}  // namespace metasearch
}  // namespace cider
