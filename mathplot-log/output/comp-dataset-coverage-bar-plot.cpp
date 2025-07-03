// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-dataset-coverage-bar-plot.h"

#include <assert.h>

#include <iostream>

#include "recorder/details/generator.h"

#ifdef ENABLE_MATHPLOT
#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;
#endif

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

CoverageBarPlot::CoverageBarPlot(const std::string& logDir,
                                 const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}
CoverageBarPlot::~CoverageBarPlot() {
  save();
}

void CoverageBarPlot::next(const std::string& label) {
  _barData.emplace_back(BarPlotData{});
  _current = &_barData.back();

  _current->label = label;
}

void CoverageBarPlot::log(size_t,
                          const gcov_coverage::RootReport& coverage) const {
  if (_current == nullptr) {
    return;
  }

  _current->report = coverage;
}

void CoverageBarPlot::save() {
  if (m_saved) {
    return;
  }
  m_saved = true;
  plt::save(ensurePngExtension(m_path), 1200);
}

void CoverageBarPlot::plot() const {
  if (_barData.empty()) {
    return;
  }

  plt::clf();

  std::vector<std::string> applications;
  std::transform(_barData.cbegin(), _barData.cend(),
                 std::back_inserter(applications),
                 [](const auto& entry) { return entry.label; });
  size_t n = applications.size();

  // Bar values
  std::vector<double> lCovs;
  std::vector<double> bCovs;
  std::vector<double> fCovs;

  std::transform(
      _barData.cbegin(), _barData.cend(), std::back_inserter(lCovs),
      [](const auto& entry) { return entry.report.report.lineCov.percent; });
  std::transform(
      _barData.cbegin(), _barData.cend(), std::back_inserter(bCovs),
      [](const auto& entry) { return entry.report.report.branchCov.percent; });
  std::transform(_barData.cbegin(), _barData.cend(), std::back_inserter(fCovs),
                 [](const auto& entry) {
                   return entry.report.report.funcCov.percent;
                   ;
                 });

  // X positions (base for each group)
  std::vector<double> x(n);
  for (size_t i = 0; i < n; ++i)
    x[i] = static_cast<double>(i);

  // Group offset (for 3 bars per group)
  std::vector<double> x1(n), x2(n), x3(n);
  double width = 0.2;

  for (size_t i = 0; i < n; ++i) {
    x1[i] = x[i] - width;
    x2[i] = x[i];
    x3[i] = x[i] + width;
  }

  plt::bar(x1, lCovs, "black", "-", 0.5, width, {{"label", "Line Coverage"}});
  plt::bar(x2, fCovs, "black", "-", 0.5, width, {{"label", "Func Coverage"}});
  plt::bar(x3, bCovs, "black", "-", 0.5, width, {{"label", "Branch Coverage"}});

  // Set ticks and labels
  plt::xticks(x, applications);
  plt::ylabel("Coverage (%)");

  plt::legend();

  plt::pause(0.01);
}

}  // namespace mathplot
}  // namespace cider
