// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace cider {
namespace mathplot {

class CoverageHeatmapPlot {
 public:
  CoverageHeatmapPlot(const std::string& logDir,
                      const std::string& logFileName);

  ~CoverageHeatmapPlot();

  void add(const std::string& methodLabel,
           const std::vector<uint8_t>& coveredBranches);

  void setRef(const std::vector<std::uint8_t>& original) {
    if (m_original.empty()) {
      m_original = original;
    }
  }

  void plot();

 private:
  std::string m_path;
  std::unordered_map<std::string, std::vector<std::vector<std::uint8_t>>>
      m_rawMatrix;
  std::vector<std::uint8_t> m_original;
};

}  // namespace mathplot
}  // namespace cider
