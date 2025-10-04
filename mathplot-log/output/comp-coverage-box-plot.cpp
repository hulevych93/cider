// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-coverage-box-plot.h"

#include <assert.h>

#include <tlog.h>
#include <sstream>

#include <math/stat-utils.h>

#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

namespace {

static std::string getYAxisName(const CoverageBoxPlot::PlotType type) {
  std::string plotName;
#ifdef ENG_NAMES
  if (type == CoverageBoxPlot::PlotType::Both) {
    plotName = "Coverage (%)";
  } else if (type == CoverageBoxPlot::PlotType::BrCov) {
    plotName = "Branch Coverage (%)";
  } else if (type == CoverageBoxPlot::PlotType::LineCov) {
    plotName = "Line Coverage (%)";
  }
#else
  if (type == CoverageBoxPlot::PlotType::Both) {
    plotName = "Покриття коду, %";
  } else if (type == CoverageBoxPlot::PlotType::BrCov) {
    plotName = "Гілкове покриття коду, %";
  } else if (type == CoverageBoxPlot::PlotType::LineCov) {
    plotName = "Лінійне покриття коду, %";
  }
#endif

  return plotName;
}

std::string getXAxisName() {
#ifdef ENG_NAMES
  return "Policy Configuration";
#else
  return "Конфігурація";
#endif
}

std::string getOriginalTSName() {
#ifdef ENG_NAMES
  return "Original TS";
#else
  return "Оригінальний ТН";
#endif
}

// === Mann–Whitney + Holm–Bonferroni support ===
struct PairResult {
  size_t i, j;
  double p_raw;
  double p_adj;
  std::string sig;
};

// Apply Holm–Bonferroni correction
std::vector<PairResult> holm_bonferroni(const std::vector<PairResult>& pairs,
                                        size_t m) {
  std::vector<PairResult> corrected = pairs;
  std::vector<size_t> order(pairs.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
    return pairs[a].p_raw < pairs[b].p_raw;
  });

  for (size_t rank = 0; rank < order.size(); ++rank) {
    size_t idx = order[rank];
    double p_adj = pairs[idx].p_raw * (m - rank);
    if (p_adj > 1.0)
      p_adj = 1.0;
    corrected[idx].p_adj = p_adj;
    corrected[idx].sig = (p_adj < 0.05) ? "*" : "ns";
  }
  return corrected;
}

void plotMannWhitney(const std::string& path,
                     double maxTop,
                     const std::vector<std::string>& labels,
                     const std::vector<std::vector<double>>& plotDatas) {
  const size_t numGroups = plotDatas.size();
  size_t m = (numGroups * (numGroups - 1)) / 2;

  if (numGroups <= 3) {
    return;
  }

  // === GLOBAL KW ===
  double p_kw = 1.0;
  try {
    p_kw = kruskal_wallis(plotDatas);
    tlog_info << "[Kruskal–Wallis] p = " << p_kw << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "Kruskal–Wallis error: " << e.what() << std::endl;
  }

  // === CSV long-format ===
  std::ofstream csv(path, std::ios::out | std::ios::trunc);
  if (!csv) {
    std::cerr << "Cannot open CSV file: " << path << std::endl;
    return;
  }
  csv << "Global_KW_p=" << p_kw << "\n";
  csv << "GroupA;GroupB;p-raw;p-holm;Significance\n";

  if (p_kw >= 0.05) {
    tlog_info << "[PostHoc] Global test not significant\n";
    return;
  }

  // --- collect pairs ---
  std::vector<PairResult> pairs;
  for (size_t i = 0; i < numGroups; ++i) {
    for (size_t j = i + 1; j < numGroups; ++j) {
      double p_raw = mann_whitney_u(plotDatas[i], plotDatas[j], "two-sided");
      pairs.push_back({i, j, p_raw, p_raw, "ns"});
    }
  }

  // --- Holm–Bonferroni ---
  pairs = holm_bonferroni(pairs, m);

  // --- Write CSV long format ---
  for (auto& pr : pairs) {
    csv << labels[pr.i] << ";" << labels[pr.j] << ";" << std::setprecision(10)
        << pr.p_raw << ";" << pr.p_adj << ";" << pr.sig << "\n";
  }
  csv.close();
  tlog_info << "[INFO] Mann–Whitney table written to " << path << std::endl;

  // --- Draw only comparisons vs first group (e.g. DSL) ---
  auto justLogOut = numGroups >= 4;
  if (!justLogOut) {
    int pairIdx = 0;
    for (auto& pr : pairs) {
      if (pr.i != 0)
        continue;  // тільки порівняння з першою групою

      double x1 = pr.i + 1;
      double x2 = pr.j + 1;
      double y = maxTop + pairIdx * 5.0;

      std::ostringstream label;
      label << "p=" << std::fixed << std::setprecision(4) << pr.p_adj << " "
            << pr.sig;

      plt::plot({x1, x1}, {y - 0.8, y},
                {{"linestyle", "-"}, {"linewidth", "0.8"}, {"color", "black"}});
      plt::plot({x2, x2}, {y - 0.8, y},
                {{"linestyle", "-"}, {"linewidth", "0.8"}, {"color", "black"}});
      plt::plot({x1, x2}, {y, y},
                {{"linestyle", "-"}, {"linewidth", "0.8"}, {"color", "black"}});
      plt::text(
          (x1 + x2) / 2.0 - 0.2, y + 0.8, label.str(),
          {{"fontname", "Helvetica"}, {"fontsize", "6"}, {"color", "black"}});
      ++pairIdx;
    }
  }
}

void exportMannWhitneyMatrix(
    const std::string& path,
    const std::vector<std::string>& labels,
    const std::vector<std::vector<double>>& plotDatas) {
  const size_t numGroups = plotDatas.size();
  size_t m = (numGroups * (numGroups - 1)) / 2;

  // === GLOBAL KW ===
  double p_kw = 1.0;
  try {
    p_kw = kruskal_wallis(plotDatas);
    tlog_info << "[Kruskal–Wallis] p = " << p_kw << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "Kruskal–Wallis error: " << e.what() << std::endl;
  }

  std::ofstream csv(path, std::ios::out | std::ios::trunc);
  if (!csv) {
    std::cerr << "Cannot open CSV file: " << path << std::endl;
    return;
  }
  csv << "Global_KW_p=" << p_kw << "\n";

  // header
  csv << "Method";
  for (const auto& label : labels)
    csv << ";" << label;
  csv << "\n";

  if (p_kw >= 0.05) {
    tlog_info << "[PostHoc] Global test not significant → skip matrix\n";
    return;
  }

  // --- collect pairs ---
  std::vector<PairResult> pairs;
  for (size_t i = 0; i < numGroups; ++i) {
    for (size_t j = i + 1; j < numGroups; ++j) {
      double p_raw = mann_whitney_u(plotDatas[i], plotDatas[j], "two-sided");
      pairs.push_back({i, j, p_raw, p_raw, "ns"});
    }
  }

  // --- Holm–Bonferroni ---
  pairs = holm_bonferroni(pairs, m);

  // --- Matrix init ---
  std::vector<std::vector<std::string>> matrix(
      numGroups, std::vector<std::string>(numGroups, "-"));

  for (auto& pr : pairs) {
    std::ostringstream cell;
    cell << std::scientific << std::setprecision(3) << pr.p_adj << " "
         << pr.sig;
    matrix[pr.i][pr.j] = cell.str();
    matrix[pr.j][pr.i] = cell.str();
  }

  // --- Write CSV ---
  for (size_t i = 0; i < numGroups; ++i) {
    csv << labels[i];
    for (size_t j = 0; j < numGroups; ++j) {
      csv << ";" << matrix[i][j];
    }
    csv << "\n";
  }
  csv.close();
  tlog_info << "[INFO] Mann–Whitney matrix written to " << path << std::endl;
}

double plotBoxStats(const std::string& path,
                    const std::vector<std::string>& labels,
                    const std::vector<std::vector<double>>& plotDatas) {
  auto justLogOut = plotDatas.size() > 4;

  int idx = 1;
  const double offsetBase = 0.23;
  double maxTop = 0;

  std::ofstream csv;
  if (justLogOut) {
    csv.open(path, std::ios::out | std::ios::trunc);
    if (csv) {
      csv << "Label;Q1;Median;Q3;Lower;Upper\n";
    }
    tlog_info << "\n[BoxStats Table]\n";
    tlog_info << "Label\tQ1\tMedian\tQ3\tLower\tUpper\n";
  }

  for (size_t i = 0; i < plotDatas.size(); ++i) {
    auto& plotData = plotDatas[i];
    const auto stats = math_stat::compute_box(plotData);

    if (stats.upper_whisker > maxTop)
      maxTop = stats.upper_whisker;

    auto add_label = [](double x, double y, double value,
                        const std::string& text) {
      std::ostringstream oss;
      oss << text << " = " << std::fixed << std::setprecision(2) << value;
      plt::text(
          x, y, oss.str(),
          {{"fontname", "Helvetica"}, {"fontsize", "7"}, {"color", "black"}});
    };

    double y = stats.q1;
    double deltaY = 0.25;  // Vertical spacing

    if (!justLogOut) {
      add_label(idx + offsetBase, y, stats.q1, "Q1");
      add_label(idx + offsetBase, y + deltaY, stats.median, "Median");
      add_label(idx + offsetBase, y + deltaY * 2, stats.q3, "Q3");
      add_label(idx + offsetBase, y + deltaY * 3, stats.lower_whisker, "Lower");
      add_label(idx + offsetBase, y + deltaY * 4, stats.upper_whisker, "Upper");
    } else {
      // Виводимо в консоль
      tlog_info << labels[i] << "\t" << std::fixed << std::setprecision(2)
                << stats.q1 << "'\t" << stats.median << "'\t" << stats.q3
                << "'\t" << stats.lower_whisker << "'\t" << stats.upper_whisker
                << "'\n";

      // Пишемо у CSV
      if (csv) {
        csv << labels[i] << ";" << std::fixed << std::setprecision(2)
            << stats.q1 << "';" << stats.median << "';" << stats.q3 << "';"
            << stats.lower_whisker << "';" << stats.upper_whisker << "'\n";
      }
    }

    ++idx;
  }

  if (csv.is_open()) {
    csv.flush();
    csv.close();
    tlog_info << "[INFO] Box stats saved to boxstats.csv\n";
  }

  return maxTop;
}

std::array<double, 2> getYAxisLims(const std::string& libName) {
  if (libName == "bitmap_cplusplus") {
    return {20.0, 32.0};
  }
  if (libName == "hjson") {
    return {30.0, 42.0};
  }
  throw std::logic_error{"Wrong library name."};
}

}  // namespace

CoverageBoxPlot::CoverageBoxPlot(const std::string& libName,
                                 const std::string& logDir,
                                 const std::string& logFileName,
                                 PlotType type)
    : _libName(libName), _type(type), m_path(ensurePath(logDir, logFileName)) {}

CoverageBoxPlot::~CoverageBoxPlot() {
  plt::save(ensureExtension(m_path, ".eps"), 1200);
  serialize(ensureExtension(m_path, ".bin"));
  plt::close();
}

void CoverageBoxPlot::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << _type;
    serializer << _boxData;
    serializer << _order;
    serializer << _originalCoverage;
    serializer.save(filePath);
  } catch (...) {
    tlog_info << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool CoverageBoxPlot::load() {
  try {
    serialization::Deserializer deserializer(ensureExtension(m_path, ".bin"));
    deserializer >> _type;
    deserializer >> _boxData;
    deserializer >> _order;
    deserializer >> _originalCoverage;
  } catch (const std::exception& e) {
    tlog_info << e.what() << std::endl;
    return false;
  }
  return true;
}

void CoverageBoxPlot::log(const std::string& label,
                          const gcov_coverage::CoverageReport& coverage) {
  std::vector<double>* current = nullptr;

  const auto it = _boxData.find(label);
  if (it != _boxData.end()) {
    auto& pointsVector = it->second;
    current = &pointsVector;
  } else {
    auto& pointsVector = _boxData[label];
    current = &pointsVector;
  }

  if (_type == PlotType::Both) {
    throw std::runtime_error{"ERR"};
  } else if (_type == PlotType::BrCov) {
    current->push_back(coverage.branchCov.percent);
  } else if (_type == PlotType::LineCov) {
    current->push_back(coverage.lineCov.percent);
  }
}

void CoverageBoxPlot::plot() {
  plt::clf();

  std::vector<double> xticks;

  std::vector<std::string> labels;
  std::vector<std::vector<double>> plotDatas;
  for (const auto& orderName : _order) {
    const auto it = _boxData.find(orderName);
    if (it == _boxData.end()) {
      tlog_info << "Warning method not simulated: " << orderName << std::endl;
      continue;
    }

    labels.emplace_back(orderName);
    plotDatas.emplace_back(it->second);
  }

  double maxTop = plotBoxStats(ensureExtension(m_path + "_stats", ".csv"),
                               labels, plotDatas);
  plotMannWhitney(ensureExtension(m_path + "_mannwhitney", ".csv"), maxTop,
                  labels, plotDatas);
  exportMannWhitneyMatrix(
      ensureExtension(m_path + "_mannwhitney_matrix", ".csv"), labels,
      plotDatas);

  xticks.resize(_boxData.size());
  for (size_t i = 0; i < _boxData.size(); ++i)
    xticks[i] = i + 1;

  // === Plot horizontal line for max original coverage ===
  double maxOriginalCoverage = _originalCoverage;

  tlog_info << maxOriginalCoverage << std::endl;

  plt::plot(
      std::vector<double>{0.5, xticks.back()},
      std::vector<double>{maxOriginalCoverage, maxOriginalCoverage},
      {{"linestyle", "-."},
       {"color",
        "#333333"},  // насичений сірий замість фіолетового{"color", "#c5b0d5"},
       {"linewidth", "1.0"},
       {"label", getOriginalTSName()}});

  int i = 1;
  for (const auto& orderName : _order) {
    const auto iter = _boxData.find(orderName);
    if (iter == _boxData.end()) {
      tlog_info << "Warning method not simulated: " << orderName << std::endl;
      continue;
    }

    plt::boxplot(std::vector<std::vector<double>>{iter->second}, {iter->first},
                 {(double)i}, true,
                 {{"patch_artist", "True"},
                  {"widths", "0.4"},
                  {"boxprops.facecolor", getColorByLabel(iter->first)},
                  {"boxprops.linewidth", "1.0"},
                  {"boxprops.edgecolor", "black"},
                  {"medianprops.color", "black"},
                  {"medianprops.linewidth", "1.5"},
                  {"whiskerprops.linewidth", "1.0"},
                  {"whiskerprops.color", "black"},
                  {"capprops.linewidth", "1.0"},
                  {"capprops.color", "black"},
                  {"flierprops.markeredgewidth", "0.8"},
                  {"flierprops.markerfacecolor", "gray"},
                  {"flierprops.marker", "o"},
                  {"flierprops.markersize", "3.5"},
                  {"showfliers", "True"}});
    ++i;
  }

  plt::xticks(xticks, labels, {{"fontsize", "8"}});

  plt::ylabel(getYAxisName(_type));
  plt::xlabel(getXAxisName());
  plt::legend();

  const auto& axisLims = getYAxisLims(_libName);
  plt::ylim(axisLims[0], axisLims[1]);

  if (_boxData.size() > 9) {
    makeLegentByGroups(getColorGroups(), {0.82, 0.43});

    plt::tight_layout();
  }

  plt::grid(true);

  if (_boxData.size() > 5) {
    rotateXTicks90();
  }

  applyPublicationStyle();
  plt::pause(0.01);
}

}  // namespace mathplot
}  // namespace cider
