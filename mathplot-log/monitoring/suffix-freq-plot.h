// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

namespace cider {
namespace mathplot {

class SuffixFreqPlot final {
 public:
  SuffixFreqPlot() = default;

  SuffixFreqPlot(const std::string& logDir, const std::string& logFileName);
  ~SuffixFreqPlot();

  void log(int suffix) const;

  void plot() const;

  void save();
  bool load(const std::string& logFile = "");

 private:
  mutable std::vector<double> suffixFreq_;
  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
