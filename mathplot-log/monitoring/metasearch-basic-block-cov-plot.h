// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "metaheuristics/metasearch.h"

namespace cider {
namespace mathplot {

class BasicBlockCovLogger : public metasearch::IResultsLogger {
 public:
  BasicBlockCovLogger(const std::string& logDir,
                      const std::string& logFileName);
  ~BasicBlockCovLogger() override;

  void log(size_t index, const metasearch::Solution& best) const override;

  void plot() const;

 private:
  mutable std::vector<double> x_, y_;
  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
