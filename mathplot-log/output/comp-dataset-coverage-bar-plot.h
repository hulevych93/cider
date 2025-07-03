// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "coverage/logger.h"

namespace cider {
namespace mathplot {

struct BarPlotData final {
  std::string label;
  gcov_coverage::RootReport report;
  size_t lines = 0;
};

class CoverageBarPlot final : public gcov_coverage::ICoverageLogger {
 public:
  CoverageBarPlot(const std::string& logDir, const std::string& logFileName);
  ~CoverageBarPlot() override;

  void log(size_t index,
           const gcov_coverage::RootReport& coverage) const override;

  void next(const std::string& label);

  void plot() const;
  void save();

 private:
  std::vector<BarPlotData> _barData;
  BarPlotData* _current = nullptr;

  std::string m_path;
  bool m_saved = false;
};

}  // namespace mathplot
}  // namespace cider
