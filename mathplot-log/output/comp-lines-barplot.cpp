// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-lines-barplot.h"

#include <assert.h>

#include <iostream>

#include "mathplot-log/utils.h"

#ifdef ENABLE_MATHPLOT
#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;
#endif

namespace cider {
namespace mathplot {

LinesBarPlot::LinesBarPlot(const std::string& logDir,
                           const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {
  plt::figure_size(980, 640);
}
LinesBarPlot::~LinesBarPlot() {
  save();
}

void LinesBarPlot::init(const std::string& label, size_t oldLines) {
  auto& data = _barData[label];
  data.label = label;
  data.oldLines = oldLines;
}

void LinesBarPlot::log(const std::string& label,
                       const std::string& method,
                       size_t newLines) {
  auto& data = _barData[label];
  if (method == "QLB2") {
    data.newLinesB2.emplace_back(newLines);
  } else if (method == "QLG2") {
    data.newLinesG2.emplace_back(newLines);
  }

  plot();
}

void LinesBarPlot::save() {
  if (m_saved) {
    return;
  }
  m_saved = true;
  plt::save(ensurePngExtension(m_path), 1200);
}

void LinesBarPlot::plot() const {
  if (_barData.empty()) {
    return;
  }

  plt::clf();

  int tI = 0;
  std::vector<std::string> testCases;
  std::transform(_barData.cbegin(), _barData.cend(),
                 std::back_inserter(testCases),
                 [&](const auto&) -> std::string {
                   if (tI % 5 == 0) {
                     return std::to_string((tI++ + 1));
                   } else {
                     tI++;
                     return "";
                   }
                 });
  size_t n = testCases.size();

  // Bar values
  std::vector<double> oldLines;
  std::vector<double> newLinesG2;
  std::vector<double> newLinesB2;

  std::transform(_barData.cbegin(), _barData.cend(),
                 std::back_inserter(oldLines),
                 [](const auto& entry) { return entry.second.oldLines; });
  std::transform(_barData.cbegin(), _barData.cend(),
                 std::back_inserter(newLinesG2), [](const auto& entry) {
                   return compute_average(entry.second.newLinesG2);
                 });
  std::transform(_barData.cbegin(), _barData.cend(),
                 std::back_inserter(newLinesB2), [](const auto& entry) {
                   return compute_average(entry.second.newLinesB2);
                 });

  // X positions (base for each group)
  std::vector<double> x(n);
  for (size_t i = 0; i < n; ++i)
    x[i] = static_cast<double>(i);

  // Group offset (for 2 bars per group)
  std::vector<double> x1(n), x2(n), x3(n);
  double width = 0.2;

  for (size_t i = 0; i < n; ++i) {
    x1[i] = x[i] - width;
    x2[i] = x[i];
    x3[i] = x[i] + width;
  }

  plt::bar(x1, oldLines, "black", "-", 0.5, width, {{"label", "Original"}});
  plt::bar(x2, newLinesG2, "black", "-", 0.5, width, {{"label", "QLG2"}});
  plt::bar(x3, newLinesB2, "black", "-", 0.5, width, {{"label", "QLB2"}});

  // Set ticks and labels
  plt::xticks(x, testCases);
  plt::ylabel("Instructions Count");
  plt::xlabel("Test Script");

  plt::legend();

  plt::pause(0.01);
}

}  // namespace mathplot
}  // namespace cider
