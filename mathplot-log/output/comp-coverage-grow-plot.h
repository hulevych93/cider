// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "coverage/logger.h"
#include "metaheuristics/metasearch.h"

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

#include "mathplot-log/output/basic-plot.h"

namespace cider {
namespace mathplot {

struct Points : serialization::SerializableTag {
  std::vector<double> instructions;

  std::vector<double> lineCov;
  std::vector<double> brCov;
};

bool serialize(const Points& obj, serialization::Serializer& serializer);
bool deserialize(Points& obj, const serialization::Deserializer& deserializer);

class StepperComparativePlot : public IBasicPlot,
                               public gcov_coverage::ICoverageLogger {
 public:
  enum class PlotType { BrCov, LineCov, Both };

  StepperComparativePlot(const std::string& libName,
                         const std::string& logDir,
                         const std::string& logFileName,
                         PlotType type = PlotType::BrCov);
  ~StepperComparativePlot() override;

  void log(size_t index, const gcov_coverage::RootReport& coverage) override;

  void setOrder(const std::vector<std::string>& order) override {
    _order = order;
  }

  void serialize(const std::string& filePath) override;

  bool load() override;

  void next(const std::string& name);

  void plot() override;

  void setOriginalCov(double cov) override { _originalCoverage = cov; }

 private:
  std::string _libName;

  PlotType _type;
  std::unordered_map<std::string, std::vector<Points>> _graphs;
  std::vector<std::string> _order;

  Points* _current = nullptr;
  double _originalCoverage = 0.0;

  std::string m_path;
  size_t _updateCounter = 0;
};

}  // namespace mathplot
}  // namespace cider
