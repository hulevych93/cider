// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

namespace cider {
namespace mathplot {

class TimesBarPlot final {
 public:
  TimesBarPlot(const std::string& logDir, const std::string& logFileName);
  ~TimesBarPlot();

  void log(const std::string& method, size_t time);

  void serialize(const std::string& filePath);

  bool load();

  void plot();

 private:
  std::unordered_map<std::string, std::vector<size_t>> _barData;

  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
