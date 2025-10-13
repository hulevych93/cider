// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "test-case.h"

#include <tlog.h>
#include <thread>

#include <assert.h>
#include <random>

#include "math/softmax.h"

namespace cider {
namespace synthesis {

bool isOverFunc(const ObjectiveValue& objValue,
                const ObjectiveValue& targetValue,
                const bool noActions) {
  const auto coverageBigger = objValue.coverage > targetValue.coverage;
  const auto coverageSame = abs(objValue.coverage - targetValue.coverage) <
                            std::numeric_limits<double>::epsilon();
  return coverageBigger || coverageSame || noActions;
}

Openers::Openers(const std::string& filePath) {
  tlog_info << "Load openers: " << load(filePath) << std::endl;
}

void Openers::update(const Candidate& cand) {
  const auto candIt = _candidates.find(cand.action);
  if (candIt != _candidates.cend()) {
    const auto& candidateInfo = candIt->second;
    if (!candidateInfo.hasUniqueBlock && cand.hasUniqueBlock &&
        _previous.has_value()) {
      auto& opener = _openers[_previous.value()];
      if (opener < cand.branchGain) {
        opener = cand.branchGain;
        tlog_info << "[Openers] name - "
                  << actionToGenericRepro(_previous.value())
                  << ", future bonus: " << cand.branchGain << std::endl;
      }
    }
  }

  _candidates[cand.action] = cand;
}

bool Openers::load(const std::string& filePath) {
  _filePath = filePath;

  try {
    serialization::Deserializer deserializer(_filePath);
    deserializer >> _openers;
  } catch (const std::exception& e) {
    tlog_error << "Failed to load openers: " << e.what() << std::endl;
    return false;
  }

  std::cout << "Openers loaded: " << std::endl;
  for (const auto& opener : _openers) {
    std::cout << actionToGenericRepro(opener.first) << " - " << opener.second
              << std::endl;
  }
  std::cout << std::endl;

  return true;
}

bool Openers::save() const {
  try {
    serialization::Serializer serializer;
    serializer << _openers;
    serializer.save(_filePath);
  } catch (...) {
    return false;
  }
  return true;
}

TestScenario::TestScenario(std::mt19937& gen,
                           const recorder::Actions& initial,
                           const ObjectiveFunction& objFunc,
                           const FineObjectiveFunction& fineObjFunc,
                           double baseline)
    : _gen(gen),
      _initialSize(initial.size()),
      _objFunc(objFunc),
      _fineObjFunc(fineObjFunc) {
  for (const auto& action : initial) {
    _availableActions.emplace(action);
  }

  _initialObjVal.coverage = baseline;

  if (_initialObjVal.coverage <= std::numeric_limits<double>::epsilon()) {
    auto objValue = _objFunc(initial);
    if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
      tlog_info << "Initial: " << objValue.coverage << std::endl;
      _initialObjVal = objValue;
    } else {
      throw std::logic_error{"Bad initial script."};
    }
  }
}

void TestScenario::add(const recorder::Action& action) {
  _actions.push_back(action);

  const auto avIt = _availableActions.find(action);
  if (avIt != _availableActions.cend()) {
    _availableActions.erase(avIt);
  }
}

void TestScenario::rollback() {
  const auto& action = _actions.back();
  _availableActions.emplace(action);

  _actions.pop_back();
}

recorder::Actions TestScenario::getResult() const {
  return _actions;
}

bool TestScenario::isValid(bool storeCoverage) const {
  const auto objValue = _objFunc(_actions);
  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
    if (storeCoverage) {
      _lastObjVal = objValue;
      std::cout << "Coverage=" << _lastObjVal.coverage << "%" << std::endl;
    }
    return true;
  }
  return false;
}

bool TestScenario::isOver() const {
  return isOverFunc(_lastObjVal, _initialObjVal, _availableActions.empty());
}

recorder::Actions TestScenario::getAvailableActions() const {
  recorder::Actions actions;
  actions.reserve(_availableActions.size());
  for (const auto& action : _availableActions) {
    actions.emplace_back(action);
  }
  return actions;
}

std::vector<Candidate> TestScenario::getCandidates(Openers& openers) const {
  std::vector<synthesis::Candidate> candidates;
  candidates.reserve(_availableActions.size());

  for (const auto& action : _availableActions) {
    synthesis::Candidate candidate;
    candidate.action = action;

    auto trial = _actions;
    trial.push_back(action);

    const auto fine = _fineObjFunc(trial);
    candidate.hasUniqueBlock = fine.hasUnique;

    candidates.push_back(candidate);
  }

  if (candidates.empty()) {
    return candidates;
  }

  std::vector<synthesis::Candidate> pool;
  for (auto& c : candidates) {
    if (openers.takeInPoolObjective(c)) {
      auto trial = _actions;
      trial.push_back(c.action);

      c.branchGain = _objFunc(trial).coverage - _lastObjVal.coverage;

      pool.push_back(c);
    }

    openers.update(c);
  }

  tlog_info << "Unique-block candidates: " << pool.size() << "/"
            << candidates.size() << "\n";

  return pool;
}

std::optional<recorder::Action> TestScenario::getRandomAction() const {
  const auto actions = getAvailableActions();
  if (actions.empty()) {
    return std::nullopt;
  }
  std::uniform_int_distribution<size_t> indexDist(0, actions.size() - 1);
  return actions[indexDist(_gen)];
}

Candidate chooseWithOpeners(std::mt19937& gen,
                            std::vector<Candidate> candidates,
                            const Openers& openers,
                            size_t top_k,
                            double temperature,
                            double lambda) {
  auto getObjective = [&](const synthesis::Candidate& c) -> double {
    return openers.getObjective(lambda, c);
  };

  std::sort(candidates.begin(), candidates.end(),
            [&](const auto& a, const auto& b) {
              return getObjective(a) > getObjective(b);
            });

  size_t k = std::min(top_k, candidates.size());
  candidates.erase(candidates.cbegin() + k, candidates.cend());

  return math_stat::softmax_choice(candidates, getObjective, temperature, gen);
}

std::string Openers::Path;

}  // namespace synthesis
}  // namespace cider
