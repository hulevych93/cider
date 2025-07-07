// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "metaheuristics/args_mutator.h"

#include <variant>

namespace cider {
namespace metasearch {

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
};

class CompositeLogger final : public IResultsLogger {
 public:
  void addLogger(std::shared_ptr<IResultsLogger> logger) {
    loggers_.emplace_back(std::move(logger));
  }

  void log(size_t iteration, const Solution& best) const override {
    for (const auto& logger : loggers_) {
      logger->log(iteration, best);
    }
  }

 private:
  std::vector<std::shared_ptr<IResultsLogger>> loggers_;
};

using ActionsCallback = std::function<recorder::Actions()>;

class IMetaSearch {
 public:
  virtual ~IMetaSearch() = default;

  virtual void initialize(const ActionsCallback& callback) = 0;

  virtual void run() = 0;

  virtual const Solution& getBest() const = 0;

  virtual void setLogger(std::unique_ptr<IResultsLogger> logger) = 0;
};

enum class InstructionsMutationStrategy { None, Shuffle };

namespace cuckoo {

struct Settings final {
  const char* configName = "CS_NAN";
  int populationSize = 10;
  double Pa = 0.25;
  size_t maxIterationsWithoutUpdates = 50U;
  size_t maxIter = 22;
  ObjectiveFunction objFunc;
  ArgsMutationStrategy strategy = ArgsMutationStrategy::ShuffleBytes;
};

}  // namespace cuckoo

namespace harmony {

struct Settings final {
  const char* configName = "HS_NAN";
  int harmonyMemorySize = 10;
  double harmonyMemoryConsiderationRate = 0.95;
  double mutationRate = 0.1;
  size_t maxIterationsWithoutUpdates = 500U;
  size_t maxIter = 1000;
  ObjectiveFunction objFunc;
  ArgsMutationStrategy strategy = ArgsMutationStrategy::ShuffleBytes;
  InstructionsMutationStrategy instructionsMutationStrategy =
      InstructionsMutationStrategy::Shuffle;
};

}  // namespace harmony

using MetaSettings = std::variant<harmony::Settings, cuckoo::Settings>;

}  // namespace metasearch
}  // namespace cider
