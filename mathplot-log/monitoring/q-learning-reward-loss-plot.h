// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "q-learning/logger.h"

namespace cider {
namespace mathplot {

class MathplotLogger : public qleaning::IResultsLogger {
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

}  // namespace mathplot
}  // namespace cider
