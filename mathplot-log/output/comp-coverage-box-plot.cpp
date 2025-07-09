// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-coverage-box-plot.h"

#include <assert.h>

#include <iostream>
#include <sstream>

#ifdef ENABLE_MATHPLOT
#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;
#endif

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

namespace {

static std::string getYAxisName(const CoverageBoxPlot::PlotType type) {
  std::string plotName;
  if (type == CoverageBoxPlot::PlotType::Both) {
    plotName = "Coverage (%)";
  } else if (type == CoverageBoxPlot::PlotType::BrCov) {
    plotName = "Branch Coverage (%)";
  } else if (type == CoverageBoxPlot::PlotType::LineCov) {
    plotName = "Line Coverage (%)";
  }
  return plotName;
}

}  // namespace

CoverageBoxPlot::CoverageBoxPlot(const std::string& logDir,
                                 const std::string& logFileName,
                                 PlotType type)
    : _type(type), m_path(ensurePath(logDir, logFileName)) {
  plt::figure_size(2140, 640);
}
CoverageBoxPlot::~CoverageBoxPlot() {
  save();
}

void CoverageBoxPlot::next(const std::string& label) {
  _boxData.emplace_back(BoxPlotData{});
  _current = &_boxData.back();

  _current->label = label;
}

void CoverageBoxPlot::log(size_t,
                          const gcov_coverage::RootReport& coverage) const {
  if (_current == nullptr) {
    return;
  }

  if (_type == PlotType::Both) {
    throw std::runtime_error{"ERR"};
  } else if (_type == PlotType::BrCov) {
    _current->data.push_back(coverage.report.branchCov.percent);
  } else if (_type == PlotType::LineCov) {
    _current->data.push_back(coverage.report.lineCov.percent);
  }
}

void CoverageBoxPlot::linesCount(size_t lines) {
  if (_current == nullptr) {
    return;
  }

  _current->lines.push_back(lines);
}

void CoverageBoxPlot::save() {
  if (m_saved) {
    return;
  }
  m_saved = true;
  plt::save(ensurePngExtension(m_path), 1200);
}

void CoverageBoxPlot::plot() const {
  plt::clf();

  std::vector<double> xticks;
  std::vector<std::string> labels;
  std::vector<std::vector<double>> plotDatas;

  std::transform(_boxData.cbegin(), _boxData.cend(),
                 std::back_inserter(plotDatas), [&labels](const auto& entry) {
                   labels.emplace_back(entry.label);
                   return entry.data;
                 });

  xticks.resize(_boxData.size());
  for (size_t i = 0; i < _boxData.size(); ++i)
    xticks[i] = i + 1;

  double maxTop = 0.0;

  const auto printStats = [&maxTop](std::vector<double>& data, double x) {
    BoxStats stats = compute_box(data);

    if (stats.upper_whisker > maxTop)
      maxTop = stats.upper_whisker;

    auto add_label = [](double x, double y, double value,
                        const std::string& text) {
      std::ostringstream oss;
      oss << text << " = " << std::fixed << std::setprecision(2) << value;
      plt::text(x, y, oss.str());
    };

    double y = stats.q1;
    double deltaY = 3.5;  // Vertical spacing

    add_label(x, y, stats.q1, "Q1");
    add_label(x, y + deltaY, stats.median, "Median");
    add_label(x, y + deltaY * 2, stats.q3, "Q3");
    add_label(x, y + deltaY * 3, stats.lower_whisker, "Lower");
    add_label(x, y + deltaY * 4, stats.upper_whisker, "Upper");

    return y + deltaY * 5;
  };

  int idx = 1;
  const double offsetBase = 0.25;
  std::vector<double> yOffsets;

  for (size_t i = 0; i < plotDatas.size(); ++i) {
    auto& plotData = plotDatas[i];
    double y = printStats(plotData, idx + offsetBase);

    yOffsets.push_back(y);
    ++idx;
  }

  // === Mann–Whitney U + Bonferroni correction ===
  const size_t numGroups = plotDatas.size();
  const size_t numTests = numGroups * (numGroups - 1) / 2;
  const double alpha = 0.05;
  const double adjusted_alpha = alpha / numTests;

  // === Pairwise significance lines ===
  const double baseHeight = maxTop + 20.0;
  const double stepHeight = 5.0;

  int pairIdx = 0;
  for (size_t i = 0; i < numGroups; ++i) {
    for (size_t j = i + 1; j < numGroups; ++j) {
      try {
        double p = mann_whitney_u(plotDatas[i], plotDatas[j], "two-sided");

        std::ostringstream label;
        label << "p = " << std::fixed << std::setprecision(4) << p;
        if (p < adjusted_alpha)
          label << " *";
        else
          label << " ns";

        double x1 = i + 1;
        double x2 = j + 1;
        double y = baseHeight + pairIdx * stepHeight;

        plt::plot({x1, x1}, {y - 0.8, y}, "k-");
        plt::plot({x2, x2}, {y - 0.8, y}, "k-");

        plt::plot({x1, x2}, {y, y}, "k-");

        plt::text((x1 + x2) / 2.0, y + 0.8, label.str());

        ++pairIdx;
      } catch (const std::exception& e) {
        std::cerr << "Mann–Whitney error: " << e.what() << std::endl;
      }
    }
  }

  plt::boxplot(plotDatas, labels, true, {{"patch_artist", "True"}});
  plt::xticks(xticks, labels);

  plt::ylabel(getYAxisName(_type));
  plt::ylim(0.0, 100.0);

  plt::grid(true);

  applyPublicationStyle();
  plt::pause(0.01);
}

}  // namespace mathplot
}  // namespace cider
