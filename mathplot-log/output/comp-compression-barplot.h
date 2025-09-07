// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

#include "mathplot-log/output/basic-plot.h"

namespace cider {
namespace mathplot {

class CompressionBarPlot final : public IBasicPlot {
 public:
  CompressionBarPlot(const std::string& logDir, const std::string& logFileName);
  ~CompressionBarPlot() override;

  void setOrder(const std::vector<std::string>& order) override {
    _order = order;
  }

  void log(const std::string& method, double coefficient);

  void serialize(const std::string& filePath) override;

  bool load() override;

  void plot() override;

 private:
  std::unordered_map<std::string, std::vector<double>> _barData;
  std::vector<std::string> _order;

  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
