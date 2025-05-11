// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <deque>

namespace cider {
namespace qleaning {

class MovingAverage {
 public:
  explicit MovingAverage(size_t window_size);

  float add_value(float value);
  float get_average() const;

 private:
  size_t window_size = 0U;
  std::deque<float> window;
  float sum = 0.0f;
};

class ExponentialMovingAverage {
 public:
  ExponentialMovingAverage(float smoothing_factor);

  float add_value(float value);
  float get_average() const;

 private:
  float smoothing_factor = 0.0f;
  float average = 0.0f;
  bool initialized = false;
};

class EpsilonGreedyAdaptor {
 public:
  EpsilonGreedyAdaptor(float initial_epsilon,
                       float decay_rate,
                       float boost_value,
                       int max_stuck_steps);

  void adapt(float current_loss);

  float get_epsilon() const { return epsilon; }

 private:
  float epsilon = 0.0f;
  float decay_rate = 0.0f;
  float boost_value = 0.0f;
  int max_stuck_steps = 0.0f;

  float previous_loss = 0.0f;
  int steps_without_improvement = 0;
};

}  // namespace qleaning
}  // namespace cider
