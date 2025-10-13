// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <random>
#include <vector>

namespace cider {
namespace math_stat {

template <typename T, typename Func>
T softmax_choice(const std::vector<T>& pool,
                 Func getObjective,
                 double temperature,
                 std::mt19937& gen) {
  std::vector<double> probs(pool.size());
  double sum = 0.0;

  for (size_t k = 0; k < pool.size(); ++k) {
    double scaled = getObjective(pool[k]) / std::max(1e-9, temperature);
    double val = std::exp(scaled);
    probs[k] = val;
    sum += val;
  }

  if (sum > 0.0) {
    for (auto& p : probs)
      p /= sum;
  } else {
    // fallback uniform if all values are numerically zero
    double uniform = 1.0 / pool.size();
    std::fill(probs.begin(), probs.end(), uniform);
  }

  std::discrete_distribution<size_t> dist(probs.begin(), probs.end());
  size_t chosen = dist(gen);
  return pool[chosen];
}

}  // namespace math_stat
}  // namespace cider
