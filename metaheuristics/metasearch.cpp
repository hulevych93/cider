// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "metasearch.h"

#ifdef ENABLE_MATHPLOT
#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;
#endif

namespace cider {
namespace metasearch {

void MathplotLogger::log(size_t index, const Solution& solution) const {
#ifdef ENABLE_MATHPLOT
  x_.push_back(static_cast<double>(index));
  y_.push_back(solution.objVal);

  plt::clf();         // Clear previous frame
  plt::plot(x_, y_);  // Plot updated points
  plt::xlabel("Iteration");
  plt::ylabel("CFG Coverage (%)");
  plt::title(" ");
  plt::grid(true);
  plt::pause(0.01);  // Allow time for GUI to update
#endif
}

void MathplotLogger::save(const std::string& path) {
#ifdef ENABLE_MATHPLOT
  plt::save(path, 1200);
#endif
}

}  // namespace metasearch
}  // namespace cider
