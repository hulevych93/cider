// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

#include <map>

namespace cider {
namespace mathplot {

class SuffixExistRatePlot final {
 public:
  SuffixExistRatePlot(const std::string& logDir,
                      const std::string& logFileName);

  static SuffixExistRatePlot& get(const std::string& logDir = "",
                                  const std::string& logFileName = "") {
    static SuffixExistRatePlot plot(logDir, logFileName);
    return plot;
  }

  void finalize();

  void next(const double configName);

  void log(size_t suffix, bool found) const;

  void plot() const;

  void save();
  bool load(const std::string& logFile = "");

  std::map<double, double> meanRates() const;

 private:
  std::vector<double>* _current = nullptr;

  mutable std::map<double, std::vector<double>> suffixExist_;
  std::string m_path;

  mutable size_t _updateCounter = 0;
};

}  // namespace mathplot
}  // namespace cider
