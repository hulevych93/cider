// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "coverage/logger.h"
#include "mathplot-log/output/basic-plot.h"

namespace cider {
namespace mathplot {

class CoverageBoxPlot final : public IBasicPlot {
 public:
  enum class PlotType { BrCov, LineCov, Both };

  CoverageBoxPlot(const std::string& libName,
                  const std::string& logDir,
                  const std::string& logFileName,
                  PlotType type = PlotType::BrCov);
  ~CoverageBoxPlot() override;

  void setOrder(const std::vector<std::string>& order) override {
    _order = order;
  }

  void serialize(const std::string& filePath) override;

  bool load() override;

  void log(const std::string& label,
           const gcov_coverage::CoverageReport& coverage);

  void plot() override;

  void setOriginalCov(double cov) override { _originalCoverage = cov; }

 private:
  std::string _libName;

  PlotType _type;
  mutable std::unordered_map<std::string, std::vector<double>> _boxData;
  std::vector<std::string> _order;

  double _originalCoverage = 0.0;
  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
