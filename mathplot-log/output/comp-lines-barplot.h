// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

#include "mathplot-log/output/basic-plot.h"

namespace cider {
namespace mathplot {

class LinesBarPlot final : public IBasicPlot {
 public:
  LinesBarPlot(const std::string& logDir, const std::string& logFileName);
  ~LinesBarPlot() override;

  void setOrder(const std::vector<std::string>& order) override {}

  void log(const std::string& label,
           const std::string& method,
           size_t oldLines,
           size_t newLines);

  void serialize(const std::string& filePath) override;

  bool load() override;

  void plot() override;

 private:
  std::unordered_map<std::string,
                     std::unordered_map<std::string, std::vector<size_t>>>
      _barData;
  mutable std::unordered_map<std::string, size_t> _oldLines;

  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
