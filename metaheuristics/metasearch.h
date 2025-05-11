// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

namespace cider {
namespace metasearch {

enum class Algorithm { HarmonySearch, CuckooSearch };

using ObjectiveFunction =
    std::function<double(const std::vector<recorder::Action>&)>;

struct Solution final {
  std::vector<recorder::Action> actions;
  double objVal = 0.0f;
};

inline Solution deepCopy(const Solution& solution) {
  Solution copy;
  copy.objVal = solution.objVal;
  copy.actions = deepCopy(solution.actions);
  return copy;
}

inline bool operator>(const Solution& left, const Solution& right) {
  return left.objVal > right.objVal;
}

inline bool operator<(const Solution& left, const Solution& right) {
  return left.objVal < right.objVal;
}

class IResultsLogger {
 public:
  virtual ~IResultsLogger() = default;

  virtual void log(size_t iteration, const Solution& best) const = 0;

  virtual void save(const std::string& path) = 0;
};

class IMetaSearch {
 public:
  virtual ~IMetaSearch() = default;

  virtual void initialize(const std::vector<recorder::Action>& actions) = 0;

  virtual void run() = 0;

  virtual const Solution& getBest() const = 0;

  virtual void setLogger(std::unique_ptr<IResultsLogger> logger) = 0;

  virtual IResultsLogger& getLogger() = 0;
};

class MathplotLogger : public IResultsLogger {
 public:
  void log(size_t index, const Solution& best) const override;

  void save(const std::string& path) override;

 private:
  mutable std::vector<double> x_, y_;
};

}  // namespace metasearch
}  // namespace cider
