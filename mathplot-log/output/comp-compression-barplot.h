// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

namespace cider {
namespace mathplot {

class CompressionBarPlot final {
 public:
  CompressionBarPlot(const std::string& logDir, const std::string& logFileName);
  ~CompressionBarPlot();

  void log(const std::string& method, double coefficient);

  void serialize(const std::string& filePath);

  bool load();

  void plot();

 private:
  std::unordered_map<std::string, std::vector<double>> _barData;

  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
