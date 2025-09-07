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
  auto targetCovered = objFunc(actionSpace).coverage;
  targetCovered -= 1.5f;

  std::vector<recorder::Action> sliced = deepCopy(actionSpace);

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

  const auto targetCoveredRecheck = objFunc(actionSpace).coverage;
  auto finalCov = objFunc(sliced).coverage;
  if (finalCov < targetCoveredRecheck) {
    std::cout << "[Slice] Recheck FAILED: final=" << finalCov
              << " < target=" << targetCoveredRecheck
              << ". Rollback to original.\n";
  }

  std::cout << "[Slice] Final length: " << sliced.size() << "\n";
  return sliced;
}

TestCase run_delta_d_slicing(const ObjectiveFunction& objFunc,
                             const std::vector<recorder::Action>& actionSpace) {
  const auto target = objFunc(actionSpace).coverage;

  TestCase current = deepCopy(actionSpace);
  size_t n = 2;

  std::cout << "[ddmin] Start: length=" << current.size()
            << ", coverage=" << target << "\n";

  while (current.size() >= 2) {
    size_t chunkSize = (current.size() + n - 1) / n;  // ceil
    bool reduced = false;

    for (size_t i = 0; i < current.size(); i += chunkSize) {
      TestCase trial = current;
      trial.erase(trial.begin() + i,
                  trial.begin() + std::min(current.size(), i + chunkSize));

      auto cov = objFunc(trial).coverage;

      std::cout << "  [ddmin] Try removing block [" << i << ":"
                << std::min(current.size(), i + chunkSize)
                << "), trial len=" << trial.size() << ", coverage=" << cov
                << "\n";

      if (cov >= target) {
        std::cout << "  [ddmin] Removing block accepted\n";
        current = std::move(trial);
        n = std::max((size_t)2, n - 1);  // зменшуємо гранулярність
        reduced = true;
        break;
      }
    }

    if (!reduced) {
      if (n >= current.size())
        break;
      n = std::min(current.size(), n * 2);  // збільшуємо гранулярність
    }
  }

  std::cout << "[ddmin] Finished: length=" << current.size() << "\n";
  return current;
}

}  // namespace dslicer
}  // namespace cider
