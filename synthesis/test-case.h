// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"

#include <random>

namespace cider {
namespace synthesis {

using ActionSet = std::unordered_set<recorder::Action,
                                     recorder::FuzzyActionHash,
                                     recorder::FuzzyEqualPred>;

using ActionOpenersMap = std::unordered_map<recorder::Action,
                                            double,
                                            recorder::FuzzyActionHash,
                                            recorder::FuzzyEqualPred>;

using ObjectiveFunction =
    std::function<ObjectiveValue(const recorder::Actions&)>;

using FineObjectiveFunction =
    std::function<FineObjectiveValue(const std::vector<recorder::Action>&)>;

struct CandidateInfo {
  bool hasUniqueBlock = false;
  double branchGain = 0.0;
};

struct Candidate final : CandidateInfo {
  recorder::Action action;
};

using CandidatesMap = std::unordered_map<recorder::Action,
                                         CandidateInfo,
                                         recorder::FuzzyActionHash,
                                         recorder::FuzzyEqualPred>;

class Openers final {
 public:
     static Openers& get() {
      if (Path.empty()) {
          throw std::logic_error{"agent path error"};
      }
    static Openers openers(Path);
    return openers;
  }

  void init() {
    _previous.reset();
    _candidates.clear();
  }

  void setWinner(const recorder::Action& winner) { _previous = winner; }

  void update(const Candidate& cand);

  bool load(const std::string& filePath);
  bool save() const;

  double getObjective(double lambda, const Candidate& c) const {
    auto objective = c.branchGain;
    const auto openerIt = _openers.find(c.action);
    if (openerIt != _openers.cend()) {
      objective += lambda * openerIt->second;
    }
    return objective;
  }

  bool takeInPoolObjective(const synthesis::Candidate& c) const {
    bool take = c.hasUniqueBlock;
    const auto openerIt = _openers.find(c.action);
    if (openerIt != _openers.cend()) {
      take |= openerIt->second > 0.0f;
    }
    return take;
  }

  static void setPath(const std::string& path) { Path = path; }

 private:
  Openers(const std::string& filePath);

  CandidatesMap _candidates;
  std::optional<recorder::Action> _previous;

  ActionOpenersMap _openers;
  std::string _filePath;

  static std::string Path;
};

class TestScenario {
 public:
  TestScenario(std::mt19937& gen,
               const recorder::Actions& initial,
               const ObjectiveFunction& objFunc,
               const FineObjectiveFunction& fineObjFunc,
               double baseline = 0.0f);
  virtual ~TestScenario() = default;

  void add(const recorder::Action& action);
  void rollback();

  size_t getSize() const { return _actions.size(); }

  recorder::Actions getResult() const;
  recorder::Actions getAvailableActions() const;

  std::optional<recorder::Action> getRandomAction() const;
  std::vector<Candidate> getCandidates(Openers& openers) const;

  bool isValid(bool storeCoverage = true) const;
  bool isOver() const;

 protected:
  std::mt19937& _gen;

  mutable recorder::Actions _actions;

  ActionSet _availableActions;
  const int _initialSize;

  ObjectiveValue _initialObjVal;
  mutable ObjectiveValue _lastObjVal;

  ObjectiveFunction _objFunc;
  FineObjectiveFunction _fineObjFunc;
};

bool isOverFunc(const ObjectiveValue& objValue,
                const ObjectiveValue& targetValue,
                const bool noActions);

Candidate chooseWithOpeners(std::mt19937& gen,
                            std::vector<Candidate> candidates,
                            const Openers& openers,
                            size_t top_k,
                            double temperature,
                            double lambda);

}  // namespace synthesis
}  // namespace cider
