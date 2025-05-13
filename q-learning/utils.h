// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <math.h>
#include <deque>
#include <iostream>

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

class AdvancedAdaptiveLearningRate final {
 public:
  AdvancedAdaptiveLearningRate(float initial_lr = 0.1,
                               float min_lr = 0.05,
                               float max_lr = 0.3,
                               float increase_factor = 1.005,
                               float decrease_factor = 0.995,
                               int patience = 2,
                               float reward_stability_threshold = 0.01)
      : learning_rate(initial_lr),
        min_lr(min_lr),
        max_lr(max_lr),
        increase_factor(increase_factor),
        decrease_factor(decrease_factor),
        patience(patience),
        previous_loss(0.0f),
        steps_without_improvement(0),
        reward_stability_threshold(reward_stability_threshold),
        is_initialized(false) {}

  void adapt(float current_loss, float current_reward) {
    if (!is_initialized) {
      previous_loss = current_loss;
      previous_reward = current_reward;
      is_initialized = true;
      return;
    }

    if (current_loss < previous_loss) {
      steps_without_improvement = 0;
      learning_rate = std::min(max_lr, learning_rate * increase_factor);
    } else {
      steps_without_improvement++;
      if (steps_without_improvement >= patience) {
        learning_rate = std::max(min_lr, learning_rate * decrease_factor);
        steps_without_improvement = 0;
        std::cout << "Learning Rate descreased: " << learning_rate << std::endl;
      }
    }

    reward_buffer.push_back(current_reward);
    if (reward_buffer.size() > 10) {
      reward_buffer.pop_front();
    }

    float average_reward = 0.0f;
    for (float r : reward_buffer) {
      average_reward += r;
    }
    average_reward /= reward_buffer.size();

    if (std::fabs(average_reward - current_reward) <
        reward_stability_threshold) {
      learning_rate = std::min(max_lr, learning_rate * 1.1f);
      std::cout << "Stable reward, incresing Learning Rate: " << learning_rate
                << std::endl;
    }

    previous_loss = current_loss;
    previous_reward = current_reward;
  }

  float get_learning_rate() const { return learning_rate; }

 private:
  float learning_rate;
  float min_lr;
  float max_lr;
  float increase_factor;
  float decrease_factor;
  int patience;
  float reward_stability_threshold;

  float previous_loss;
  float previous_reward;

  int steps_without_improvement;
  bool is_initialized;

  std::deque<float> reward_buffer;
};

}  // namespace qleaning
}  // namespace cider
