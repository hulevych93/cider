// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

#include "mathplot-log/output/basic-plot.h"

namespace cider {
namespace mathplot {

class TimesBarPlot final : public IBasicPlot {
 public:
  TimesBarPlot(const std::string& logDir, const std::string& logFileName);
  ~TimesBarPlot() override;

  void setOrder(const std::vector<std::string>& order) override {
    _order = order;
  }

  void log(const std::string& method, size_t time);

  void serialize(const std::string& filePath) override;

  bool load() override;

  void plot() override;

 private:
  std::unordered_map<std::string, std::vector<size_t>> _barData;
  std::vector<std::string> _order;

  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
