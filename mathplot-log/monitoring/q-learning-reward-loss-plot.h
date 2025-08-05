// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "agent-model/logger.h"
#include "serialization/deserializer.h"
#include "serialization/serializer.h"

namespace cider {
namespace mathplot {

class QLearningRewardLogger : public agent_model::IRewardLogger {
 public:
  QLearningRewardLogger(const std::string& logDir,
                        const std::string& logFileName);
  ~QLearningRewardLogger() override;

  void log(size_t episode, const double totalReward) override;

  void serialize(const std::string& filePath);

  bool load();

  void plot();

  void save() override;

 private:
  mutable std::vector<double> ieps_, rwrd_;
  std::string m_path;
};

class QLearningLossLogger : public agent_model::ILossLogger {
 public:
  QLearningLossLogger(const std::string& logDir,
                      const std::string& logFileName);
  ~QLearningLossLogger() override;

  void log(size_t episode, const double averageLoss) override;

  void serialize(const std::string& filePath);

  bool load();

  void plot();

  void save() override;

 private:
  mutable std::vector<double> jeps_, loss_;

  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
