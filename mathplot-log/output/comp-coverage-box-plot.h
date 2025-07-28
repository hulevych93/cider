// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "coverage/logger.h"

namespace cider {
namespace mathplot {

struct BoxPlotData final {
  std::string label;
  std::vector<double> data;
  std::vector<size_t> lines;
};

class CoverageBoxPlot final : public gcov_coverage::ICoverageLogger {
 public:
  enum class PlotType { BrCov, LineCov, Both };

  CoverageBoxPlot(const std::string& logDir,
                  const std::string& logFileName,
                  PlotType type = PlotType::BrCov);
  ~CoverageBoxPlot() override;

  void log(size_t index,
           const gcov_coverage::RootReport& coverage) const override;

  void next(const std::string& label);

  void linesCount(size_t lines);

  void plot() const;

 private:
  PlotType _type;
  std::vector<BoxPlotData> _boxData;
  BoxPlotData* _current = nullptr;

  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
