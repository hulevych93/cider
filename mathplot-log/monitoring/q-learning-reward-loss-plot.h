// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "agent-model/logger.h"
#include "serialization/deserializer.h"
#include "serialization/serializer.h"

namespace cider {
namespace mathplot {

class QLearningResultsMathplotLogger : public agent_model::IResultsLogger {
 public:
  enum PlotType { Loss, Reward };

  QLearningResultsMathplotLogger(const std::string& logDir,
                                 const std::string& logFileName);
  ~QLearningResultsMathplotLogger() override;

  void logReward(size_t episode, const double totalReward) override;
  void logLoss(size_t episode, const double averageLoss) override;

  void serialize(const std::string& filePath);

  bool load();

  void plot();

  void save() override;

 private:
  PlotType _plot;
  mutable std::vector<double> ieps_, rwrd_;
  mutable std::vector<double> jeps_, loss_;

  std::string m_path;
  mutable size_t _updateCounter = 0;
};

}  // namespace mathplot
}  // namespace cider
