// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

#include "mathplot-log/output/basic-plot.h"

namespace cider {
namespace mathplot {

class CoverageHeatmapPlot : public IBasicPlot {
 public:
  CoverageHeatmapPlot(const std::string& logDir,
                      const std::string& logFileName);
  ~CoverageHeatmapPlot() override;

  virtual void setOrder(const std::vector<std::string>& order) override {
    _order = order;
  }

  virtual void serialize(const std::string& filePath) override;

  virtual bool load() override;

  void add(const std::string& methodLabel,
           const std::vector<uint8_t>& coveredBranches);

  void setRef(const std::vector<std::uint8_t>& original) {
    if (_original.empty()) {
      _original = original;
    }
  }

  void plot() override;

 private:
  std::string m_path;

  std::unordered_map<std::string, std::vector<std::vector<std::uint8_t>>>
      _rawMatrix;
  std::vector<std::string> _order;

  std::vector<std::uint8_t> _original;
};

}  // namespace mathplot
}  // namespace cider
