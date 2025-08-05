// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <fstream>

namespace cider {
namespace agent_model {

class IRewardLogger {
 public:
  virtual ~IRewardLogger() = default;

  virtual void log(size_t episode, double totalReward) = 0;

  virtual void save() = 0;
};

class ILossLogger {
 public:
  virtual ~ILossLogger() = default;

  virtual void log(size_t episode, double averageLoss) = 0;

  virtual void save() = 0;
};

class ICoverageLogger {
 public:
  virtual ~ICoverageLogger() = default;

  virtual void set(double maxCov) = 0;

  virtual void log(size_t episode, double cov) = 0;

  virtual void save() = 0;
};

}  // namespace agent_model
}  // namespace cider
