// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"

namespace cider {
namespace harmony {

using ReportOpt = std::optional<coverage::RootReport>;
using MeassureCallback =
    std::function<ReportOpt(const std::vector<recorder::Action>&)>;

enum class MutationStrategy { ShuffleBytes, ChangeBits };

struct Settings final {
  int harmonyMemorySize = 10;
  double harmonyMemoryConsiderationRate = 0.95;
  double mutationRate = 0.1;
  size_t maxIterations = 500U;
  MeassureCallback meassure;
  MutationStrategy strategy = MutationStrategy::ShuffleBytes;
};

struct Harmony final {
  std::vector<recorder::Action> actions;
  coverage::RootReport cov;
};

class Search final {
 public:
  explicit Search(const Settings& settings);

  void initialize(const std::vector<recorder::Action>& actions);

  void run();

  const Harmony& getBest() const;

 private:
  Harmony generateHarmony(const Harmony& harmony) const;
  Harmony mutateHarmony(const Harmony& harmony) const;
  void updateHarmonyMemory(const Harmony& harmony);

  void dump();

 private:
  Settings _settings;
  std::vector<Harmony> _harmonyMemory;
  Harmony _initial;
  std::unique_ptr<recorder::IParamMutator> _mutator;
};

std::unique_ptr<recorder::IParamMutator> makeMutator(double mutationRate,
                                                     MutationStrategy strategy);

}  // namespace harmony
}  // namespace cider
