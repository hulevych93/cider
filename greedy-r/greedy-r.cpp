// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "greedy-r.h"

#include "synthesis/synthesis.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_set>
#include <vector>

namespace cider {
namespace greedy_r {

std::ostream& operator<<(std::ostream& os, const GreedyRSettings& settings) {
  os << "GREEDY_R_";
  os << "top_k[";
  os << settings.top_k;
  os << "]_maxZeroGain[";
  os << settings.maxZeroGain;
  os << "]";
  return os;
}

using TestCase = std::vector<recorder::Action>;

// Randomized Greedy Minimizer
TestCase run_greedy_r(std::mt19937& gen,
                      const GreedyRSettings& settings,
                      const std::vector<recorder::Action>& actionSpace) {
  const auto fullCoverage = settings.objFunc(actionSpace).coverage;

  std::vector<recorder::Action> selected;
  std::vector<bool> used(actionSpace.size(), false);
  double currentCoverage = 0.0;

  std::cout << "[Greedy-R] Target covered tracks: " << fullCoverage << "\n";

  size_t step = 0;
  size_t zeroGain = 0;

  while (true) {
    struct Candidate {
      size_t index = 0;
      double gain = 0;
      double coverage = 0;
    };
    std::vector<Candidate> candidates;
    std::vector<Candidate> others;

    for (size_t i = 0; i < actionSpace.size(); ++i) {
      if (used[i])
        continue;

      auto trial = selected;
      trial.push_back(actionSpace[i]);
      auto cov = settings.objFunc(trial).coverage;

      double gain = cov - currentCoverage;
      if (gain > 0) {
        std::cout << "  [Trial] Action #" << i << ", gain = " << gain << "\n";
        candidates.push_back({i, gain, cov});
      } else {
        others.push_back({i, gain, cov});
      }
    }

    if (candidates.empty() && zeroGain >= settings.maxZeroGain) {
      std::cout << "[Greedy-R] No further gain, stopping.\n";
      break;
    }

    if (candidates.empty() && others.empty()) {
      std::cout << "[Greedy-R] No further actions, stopping.\n";
      break;
    }

    auto getWinner = [&]() {
      if (!candidates.empty()) {
        std::sort(candidates.begin(), candidates.end(),
                  [](const Candidate& a, const Candidate& b) {
                    return a.gain > b.gain;
                  });

        size_t k = std::min(settings.top_k, candidates.size());
        std::uniform_int_distribution<size_t> dist(0, k - 1);
        size_t chosen = dist(gen);

        return candidates[chosen];
      } else {
        size_t k = std::min(settings.top_k, others.size());
        std::uniform_int_distribution<size_t> dist(0, k - 1);
        size_t chosen = dist(gen);
        zeroGain++;

        return others[chosen];
      }
    };

    const auto& winner = getWinner();
    used[winner.index] = true;
    selected.push_back(actionSpace[winner.index]);
    currentCoverage = winner.coverage;

    ++step;

    std::cout << "[Greedy-R] Step " << step << ": selected action #"
              << winner.index << ", gain = " << winner.gain
              << ", total covered = " << currentCoverage << "/" << fullCoverage
              << "\n";

    if (currentCoverage >= fullCoverage) {
      std::cout << "[Greedy-R] Target coverage reached.\n";
      break;
    }
  }

  std::cout << "[Greedy-R] Final script length: " << selected.size() << "\n";
  return selected;
}

}  // namespace greedy_r
}  // namespace cider
