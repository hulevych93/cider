// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "dynamic-slicer.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_set>
#include <vector>

namespace cider {
namespace dslicer {

using TestCase = std::vector<recorder::Action>;

TestCase run_d_slicing(const ObjectiveFunction& objFunc,
                       const std::vector<recorder::Action>& actionSpace) {
  const auto targetCovered = objFunc(actionSpace).coverage;

  std::vector<recorder::Action> sliced = actionSpace;

  std::cout << "[Slice] Original length: " << actionSpace.size()
            << ", branches covered %: " << targetCovered << "\n";

  for (size_t i = 0; i < sliced.size();) {
    std::vector<recorder::Action> trial = sliced;
    trial.erase(trial.begin() + i);

    auto cov = objFunc(trial).coverage;

    if (cov >= targetCovered) {
      std::cout << "  [Slice] Removing action #" << i << " → OK\n"
                << ", total covered = " << cov << "/" << targetCovered << "\n";
      sliced = std::move(trial);
    } else {
      ++i;
    }
  }

  std::cout << "[Slice] Final length: " << sliced.size() << "\n";
  return sliced;
}

}  // namespace dslicer
}  // namespace cider
