// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

namespace cider {
namespace mathplot {

class LinesBarPlot final {
 public:
  LinesBarPlot(const std::string& logDir, const std::string& logFileName);
  ~LinesBarPlot();

  void log(const std::string& label,
           const std::string& method,
           size_t oldLines,
           size_t newLines);

  void serialize(const std::string& filePath);

  bool load();

  void plot();

 private:
  std::unordered_map<std::string,
                     std::unordered_map<std::string, std::vector<size_t>>>
      _barData;
  mutable std::unordered_map<std::string, size_t> _oldLines;

  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
