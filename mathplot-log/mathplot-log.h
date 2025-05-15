// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/logger.h"
#include "metaheuristics/metasearch.h"

#include "q-learning/logger.h"

namespace cider {

namespace cfg_coverage {

class MathplotLogger : public ICoverageLogger {
 public:
  MathplotLogger(const std::string& logDir, const std::string& logFileName);
  ~MathplotLogger() override;

  void log(size_t index, const Coverage& coverage) const override;

  void plot() const;

 private:
  mutable std::vector<double> index_, _percents;
  std::string m_path;
};

}  // namespace cfg_coverage

namespace gcov_coverage {

class MathplotLogger : public ICoverageLogger {
 public:
  MathplotLogger(const std::string& logDir, const std::string& logFileName);
  ~MathplotLogger() override;

  void log(size_t index, const RootReport& coverage) const override;

  void plot() const;

 private:
  mutable std::vector<double> i_, lcov_, bcov_, fcov_;
  std::string m_path;
};

class ComparativeLogger : public ICoverageLogger {
 public:
  ComparativeLogger(const std::string& logDir, const std::string& logFileName);
  ~ComparativeLogger() override;

  void log(size_t index, const RootReport& coverage) const override;

  void first() { m_first = true; }
  void other() { m_first = false; }

  void plot() const;

 private:
  mutable std::vector<double> i_, ilcov_, ibcov_;
  mutable std::vector<double> j_, jlcov_, jbcov_;

  bool m_first = true;

  std::string m_path;
};

class TripleComparativeLogger : public ICoverageLogger {
 public:
  TripleComparativeLogger(const std::string& logDir,
                          const std::string& logFileName);
  ~TripleComparativeLogger() override;

  void log(size_t index, const RootReport& coverage) const override;

  void md(size_t md) { _md = md; }

  void plot() const;

 private:
  mutable std::vector<double> i_, ibcov_;
  mutable std::vector<double> j_, jbcov_;
  mutable std::vector<double> k_, kbcov_;

  size_t _md = 0;

  std::string m_path;
};

}  // namespace gcov_coverage

namespace metasearch {

class MathplotLogger : public IResultsLogger {
 public:
  MathplotLogger(const std::string& logDir, const std::string& logFileName);
  ~MathplotLogger() override;

  void log(size_t index, const Solution& best) const override;

  void plot() const;

 private:
  mutable std::vector<double> x_, y_;
  std::string m_path;
};

}  // namespace metasearch

namespace qleaning {

class MathplotLogger : public IResultsLogger {
 public:
  MathplotLogger(const std::string& logDir, const std::string& logFileName);
  ~MathplotLogger() override;

  void logReward(size_t episode, const double totalReward) const override;
  void logLoss(size_t episode, const double averageLoss) const override;

  void plot() const;

 private:
  mutable std::vector<double> ieps_, rwrd_;
  mutable std::vector<double> jeps_, loss_;

  std::string m_path;
  mutable size_t _updateCounter = 0;
};

}  // namespace qleaning

}  // namespace cider
