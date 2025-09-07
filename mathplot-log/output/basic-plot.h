// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

namespace cider {
namespace mathplot {

class IBasicPlot {
 public:
  virtual ~IBasicPlot() = default;

  virtual void setOrder(const std::vector<std::string>& order) = 0;

  virtual void serialize(const std::string& filePath) = 0;

  virtual bool load() = 0;

  virtual void plot() = 0;

  virtual void setOriginalCov(double /*cov*/) {}
};

}  // namespace mathplot
}  // namespace cider
