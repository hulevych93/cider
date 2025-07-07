// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "metasearch-basic-block-cov-plot.h"

#include <assert.h>

#include <iostream>

#ifdef ENABLE_MATHPLOT
#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;
#endif

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

BasicBlockCovLogger::BasicBlockCovLogger(const std::string& logDir,
                                         const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

void BasicBlockCovLogger::log(size_t index,
                              const metasearch::Solution& solution) const {
  x_.push_back(static_cast<double>(index));
  y_.push_back(solution.objVal);

  plot();
}

void BasicBlockCovLogger::plot() const {
  plt::clf();         // Clear previous frame
  plt::plot(x_, y_);  // Plot updated points
  plt::xlabel("Iteration");
  plt::ylabel("Basic Block Coverage (%) ");
  plt::title(" ");
  plt::grid(true);
  plt::pause(0.01);  // Allow time for GUI to update
}

BasicBlockCovLogger::~BasicBlockCovLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();
}

}  // namespace mathplot
}  // namespace cider
