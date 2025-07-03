// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

namespace cider {
namespace mathplot {

struct LinesBarPlotData final {
  std::string label;
  size_t oldLines = 0;
  std::vector<size_t> newLinesG2;
  std::vector<size_t> newLinesB2;
};

class LinesBarPlot final {
 public:
  LinesBarPlot(const std::string& logDir, const std::string& logFileName);
  ~LinesBarPlot();

  void init(const std::string& label, size_t oldLines);
  void log(const std::string& label,
           const std::string& method,
           size_t newLines);

  void plot() const;
  void save();

 private:
  std::unordered_map<std::string, LinesBarPlotData> _barData;

  std::string m_path;
  bool m_saved = false;
};

}  // namespace mathplot
}  // namespace cider
