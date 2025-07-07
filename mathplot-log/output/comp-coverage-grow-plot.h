// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/logger.h"
#include "metaheuristics/metasearch.h"

#include "q-learning/logger.h"

namespace cider {
namespace mathplot {

struct Points {
  std::vector<double> instructions;

  std::vector<double> lineCov;
  std::vector<double> brCov;
};

class StepperComparativeLogger : public gcov_coverage::ICoverageLogger {
 public:
  enum class PlotType { BrCov, LineCov, Both };

  StepperComparativeLogger(const std::string& logDir,
                           const std::string& logFileName,
                           PlotType type = PlotType::BrCov);
  ~StepperComparativeLogger() override;

  void log(size_t index,
           const gcov_coverage::RootReport& coverage) const override;

  void next(const std::string& name);

  void plot() const;

 private:
  PlotType _type;
  mutable std::unordered_map<std::string, std::vector<Points>> _graphs;
  std::vector<std::string> _order;
  Points* _current = nullptr;
  int _style = 0;
  int _color = 0;

  std::string m_path;
  mutable size_t _updateCounter = 0;
};

}  // namespace mathplot
}  // namespace cider
