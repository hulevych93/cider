// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "cuckoo.h"

#include <assert.h>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>

namespace cider {
namespace metasearch {
namespace cuckoo {

namespace {

template <typename ContainerType, typename Func>
void levyFlight(std::mt19937& gen, ContainerType& container, Func&& func) {
  if (container.empty()) {
    return;
  }

  std::uniform_int_distribution<> dist(0, container.size() - 1);
  std::normal_distribution<> levy_dist(1, 3);

  const int num_mutations = std::max(1, static_cast<int>(abs(levy_dist(gen))));

  if (num_mutations == 0) {
    return;
  }

  std::vector<int> ids;
  ids.reserve(num_mutations);

  int i = dist(gen);
  ids.emplace_back(i);

  for (int count = 0; count < num_mutations - 1; ++count) {
    int step = std::max(1, static_cast<int>(abs(levy_dist(gen))));
    i = (i + step) % container.size();
    ids.emplace_back(i);
  }

  for (const auto& id : ids) {
    func(container[id]);
  }
}

struct ActionMutator final {
  explicit ActionMutator(std::mt19937& gen,
                         const recorder::IParamMutator& mutator)
      : _gen(gen), _mutator(mutator) {}

  void operator()(recorder::Function& context) {
    levyFlight(_gen, context.params,
               [&](auto& param) { std::visit(_mutator, param); });
  }

  void operator()(recorder::ClassMethod& context) {
    levyFlight(_gen, context.method.params,
               [&](auto& param) { std::visit(_mutator, param); });
  }

  void operator()(recorder::ClassBinaryOp& context) {
    std::visit(_mutator, context.param);
  }

  void operator()(recorder::ClassUnaryOp&) {}
  void operator()(recorder::ClassDestructor&) {}

  const recorder::IParamMutator& _mutator;
  std::mt19937& _gen;
};

}  // namespace

Search::Search(const Settings& settings)
    : _gen(_rd()),
      _settings(settings),
      _mutator(cider::metasearch::makeMutator(_gen, 1.0f, settings.strategy)) {}

void Search::initialize(const std::vector<recorder::Action>& actions) {
  _initial.actions = actions;

  if (const auto& covOpt = _settings.meassure(_initial.actions)) {
    _initial.cov = covOpt.value();
  } else {
    throw std::logic_error{"Bad initial script."};
  }

  _memory.resize(_settings.populationSize);
  for (auto i = 0U; i < _settings.populationSize; ++i) {
    _memory[i] = _initial;
  }

  dump();
}

void Search::run() {
  size_t iterWithoutUpdates = 0U;
  for (int iteration = 0;
       iterWithoutUpdates <= _settings.maxIterationsWithoutUpdates;
       ++iteration, ++iterWithoutUpdates) {
    std::cout << "Iter: " << iteration << std::endl;

    for (int i = 0; i < _memory.size(); ++i) {
      auto& nest = _memory[i];
      auto newNest = generateNest(nest);
      if (newNest.has_value()) {
        std::cout << "Candidate: ";
        cider::coverage::printTableEntry(std::cout, i, newNest->cov.report);
        if (newNest->cov > nest.cov) {
          std::cout << " <- ";
          cider::coverage::printTableEntry(std::cout, i, nest.cov.report);
          nest = *newNest;
          iterWithoutUpdates = 0U;
        }
      }
    }

    std::sort(_memory.begin(), _memory.end(),
              [](const auto& a, const auto& b) { return a.cov > b.cov; });

    for (int i = _settings.populationSize -
                 int(_settings.Pa * _settings.populationSize);
         i < _settings.populationSize; ++i) {
      _memory[i] = _initial;
    }
  }
  dump();
}

std::optional<Nest> Search::generateNest(const Nest& nest) const {
  Nest newNest = nest;

  ActionMutator mutator(_gen, *_mutator);
  levyFlight(_gen, newNest.actions,
             [&](auto& action) { std::visit(mutator, action); });

  if (const auto& covOpt = _settings.meassure(newNest.actions)) {
    newNest.cov = covOpt.value();
    return newNest;
  }

  return std::nullopt;
}

void printTableEntry(std::ostream& ss,
                     const cider::coverage::CoverageReport& report) {
  ss << report.lineCov.percent << "\t" << report.branchCov.percent << "\t"
     << report.funcCov.percent << std::endl;
}

const Nest& Search::getBest() const {
  const auto& best = *std::max_element(_memory.begin(), _memory.end());
  std::cout << "Best :";
  printTableEntry(std::cout, best.cov.report);
  return best;
}

void Search::dump() {
  for (const auto& cuckoo : _memory) {
    printTableEntry(std::cout, cuckoo.cov.report);
  }
}

}  // namespace cuckoo
}  // namespace metasearch
}  // namespace cider
