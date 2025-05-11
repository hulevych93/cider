// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "utils.h"

namespace cider {
namespace qleaning {

MovingAverage::MovingAverage(size_t window_size)
    : window_size(window_size), sum(0.0f) {}

float MovingAverage::add_value(float value) {
  window.push_back(value);
  sum += value;

  if (window.size() > window_size) {
    sum -= window.front();
    window.pop_front();
  }

  return get_average();
}

float MovingAverage::get_average() const {
  if (window.empty())
    return 0.0f;
  return sum / window.size();
}

ExponentialMovingAverage::ExponentialMovingAverage(float smoothing_factor)
    : smoothing_factor(smoothing_factor), average(0.0f), initialized(false) {}

float ExponentialMovingAverage::add_value(float value) {
  if (!initialized) {
    average = value;
    initialized = true;
  } else {
    average = (smoothing_factor * value) + (1.0f - smoothing_factor) * average;
  }
  return average;
}

float ExponentialMovingAverage::get_average() const {
  return average;
}

EpsilonGreedyAdaptor::EpsilonGreedyAdaptor(float initial_epsilon,
                                           float decay_rate,
                                           float boost_value,
                                           int max_stuck_steps)
    : epsilon(initial_epsilon),
      decay_rate(decay_rate),
      boost_value(boost_value),
      max_stuck_steps(max_stuck_steps),
      previous_loss(0.0f),
      steps_without_improvement(0) {}

void EpsilonGreedyAdaptor::adapt(float current_loss) {
  if (current_loss > previous_loss) {
    steps_without_improvement++;

    if (steps_without_improvement >= max_stuck_steps) {
      epsilon = std::min(1.0f, epsilon + boost_value);
      steps_without_improvement = 0;
    }
  } else {
    steps_without_improvement = 0;
    epsilon *= decay_rate;
    if (epsilon < 0.3f)
      epsilon = 0.3f;
  }

  previous_loss = current_loss;
}

}  // namespace qleaning
}  // namespace cider
