// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "agent-model/logger.h"
#include "serialization/deserializer.h"
#include "serialization/serializer.h"

namespace cider {
namespace mathplot {

class CovQLearningResultsMathplotLogger : public agent_model::ICovLogger {
 public:

  CovQLearningResultsMathplotLogger(const std::string& logDir,
                                    const std::string& logFileName);
  ~CovQLearningResultsMathplotLogger();

  void log(size_t episode, const double coverage) override;

  void serialize(const std::string& filePath);
  void set(double maxCov) { _maxCov = maxCov; }

  bool load();
  void save() override {}

  void plot();

 private:
     double _maxCov = 0;
  mutable std::vector<double> ieps_, cov_;

  std::string m_path;
  mutable size_t _updateCounter = 0;
};

}  // namespace mathplot
}  // namespace cider
