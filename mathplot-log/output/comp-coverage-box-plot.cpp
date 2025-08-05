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

void plotMannWhitney(double maxTop,
                     const std::vector<std::string>& labels,
                     const std::vector<std::vector<double>>& plotDatas) {
  auto justLogOut = plotDatas.size() >= 4;

  std::string linestyle = "-";
  std::string linestyleWidth = "0.8";

  // === Pairwise significance lines ===
  const double baseHeight = maxTop + 20.0;
  const double stepHeight = 5.0;

  // === Mann–Whitney U + Bonferroni correction ===
  const size_t numGroups = plotDatas.size();
  const double alpha = 0.05;
  const double adjusted_alpha = alpha;

  int pairIdx = 0;
  int i = 0;
  for (size_t j = i + 1; j < numGroups; ++j) {
    try {
      double p = mann_whitney_u(plotDatas[i], plotDatas[j], "two-sided");
      std::cout << labels[i] << " vs " << labels[j] << "p = " << std::fixed
                << std::setprecision(4) << p << std::endl;

      std::ostringstream label;
      label << "p = " << std::fixed << std::setprecision(4) << p;
      if (p < adjusted_alpha)
        label << " *";
      else
        label << " ns";

      double x1 = i + 1;
      double x2 = j + 1;
      double y = baseHeight + pairIdx * stepHeight;

      if (!justLogOut) {
        plt::plot({x1, x1}, {y - 0.8, y},
                  {{"linestyle", linestyle.c_str()},
                   {"linewidth", linestyleWidth},
                   {"color", ColorCodes[0]}});
        plt::plot({x2, x2}, {y - 0.8, y},
                  {{"linestyle", linestyle.c_str()},
                   {"linewidth", linestyleWidth},
                   {"color", ColorCodes[0]}});

        plt::plot({x1, x2}, {y, y},
                  {{"linestyle", linestyle.c_str()},
                   {"linewidth", linestyleWidth},
                   {"color", ColorCodes[0]}});

        plt::text(
            (x1 + x2) / 2.0, y + 0.8, label.str(),
            {{"fontname", "Helvetica"}, {"fontsize", "6"}, {"color", "black"}});
      }

      ++pairIdx;
    } catch (const std::exception& e) {
      std::cerr << "Mann–Whitney error: " << e.what() << std::endl;
    }
  }
}

double plotBoxStats(const std::vector<std::vector<double>>& plotDatas) {
  auto justLogOut = plotDatas.size() >= 40;

  int idx = 1;
  const double offsetBase = 0.3;
  double maxTop = 0;

  for (size_t i = 0; i < plotDatas.size(); ++i) {
    auto& plotData = plotDatas[i];

    BoxStats stats = compute_box(plotData);

    if (stats.upper_whisker > maxTop)
      maxTop = stats.upper_whisker;

    auto add_label = [](double x, double y, double value,
                        const std::string& text) {
      std::ostringstream oss;
      oss << text << " = " << std::fixed << std::setprecision(2) << value;
      plt::text(
          x, y, oss.str(),
          {{"fontname", "Helvetica"}, {"fontsize", "5"}, {"color", "black"}});
    };

    double y = stats.q1;
    double deltaY = 1.5;  // Vertical spacing

    if (!justLogOut) {
      add_label(idx + offsetBase, y, stats.q1, "Q1");
      add_label(idx + offsetBase, y + deltaY, stats.median, "Median");
      add_label(idx + offsetBase, y + deltaY * 2, stats.q3, "Q3");
      add_label(idx + offsetBase, y + deltaY * 3, stats.lower_whisker, "Lower");
      add_label(idx + offsetBase, y + deltaY * 4, stats.upper_whisker, "Upper");
    }

    ++idx;
  }

  return maxTop;
}

}  // namespace

CoverageBoxPlot::CoverageBoxPlot(const std::string& logDir,
                                 const std::string& logFileName,
                                 PlotType type)
    : _type(type), m_path(ensurePath(logDir, logFileName)) {}

CoverageBoxPlot::~CoverageBoxPlot() {
  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();
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

  double maxTop = plotBoxStats(plotDatas);
  plotMannWhitney(maxTop, labels, plotDatas);

  plt::boxplot(plotDatas, labels, true,
               {{"patch_artist", "True"},
                {"boxprops.linewidth", "0.6"},
                {"capprops.linewidth", "0.6"},
                {"whiskerprops.linewidth", "0.6"},
                {"medianprops.linewidth", "0.6"},
                {"flierprops.markeredgewidth", "0.8"},
                {"flierprops.marker", "o"},
                {"flierprops.markersize", "3.0"},
                {"showfliers", "True"}});

  plt::xticks(xticks, labels);

  plt::ylabel(getYAxisName(_type));
  plt::ylim(0.0, 40.0);

  plt::grid(true);

  applyPublicationStyle();
  plt::pause(0.01);
}

}  // namespace mathplot
}  // namespace cider
