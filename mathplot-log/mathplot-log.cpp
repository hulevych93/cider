// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "mathplot-log.h"

#include <assert.h>

#include <iostream>

#include "recorder/details/generator.h"

#ifdef ENABLE_MATHPLOT
#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;
#endif

namespace cider {

namespace {

struct BoxStats {
  double q1, median, q3, iqr, mean = 0;
  double lower_whisker, upper_whisker;
  std::vector<double> outliers;
};

BoxStats compute_box(std::vector<double> data) {
  BoxStats s;
  if (data.empty())
    return s;

  std::sort(data.begin(), data.end());
  const size_t n = data.size();

  // Interpolated percentile
  auto percentile = [&](double p) -> double {
    double pos = p * (n - 1);
    size_t i0 = static_cast<size_t>(std::floor(pos));
    size_t i1 = std::min(i0 + 1, n - 1);
    double frac = pos - i0;
    return data[i0] + frac * (data[i1] - data[i0]);
  };

  s.q1 = percentile(0.25);
  s.median = percentile(0.5);
  s.q3 = percentile(0.75);
  s.iqr = s.q3 - s.q1;

  double lower_bound = s.q1 - 1.5 * s.iqr;
  double upper_bound = s.q3 + 1.5 * s.iqr;

  // Mean
  double sum = 0.0;
  for (double v : data)
    sum += v;
  s.mean = sum / static_cast<double>(n);

  // Whiskers are the most extreme values within the bounds
  for (double v : data) {
    if (v >= lower_bound) {
      s.lower_whisker = v;
      break;
    }
  }

  for (auto it = data.rbegin(); it != data.rend(); ++it) {
    if (*it <= upper_bound) {
      s.upper_whisker = *it;
      break;
    }
  }

  // Outliers are outside the whiskers
  for (double v : data) {
    if (v < s.lower_whisker || v > s.upper_whisker) {
      s.outliers.push_back(v);
    }
  }

  return s;
}

double compute_average(const std::vector<size_t>& vec) {
  if (vec.empty())
    return 0.0;

  // Use unsigned long long to safely hold the sum
  unsigned long long sum = std::accumulate(vec.begin(), vec.end(), 0ULL);

  return static_cast<double>(sum) / vec.size();
}

std::string ensurePngExtension(const std::string& path) {
  std::filesystem::path filePath(path);

  // Check if the extension is already .png (case insensitive)
  if (filePath.extension() == ".png") {
    return filePath.string();
  }

  // Add .png extension
  filePath.replace_extension(".png");
  return filePath.string();
}

std::string ensurePath(const std::string& logDir,
                       const std::string& logFileName) {
  std::filesystem::path outPath(logDir);
  std::filesystem::create_directories(outPath);
  outPath /= logFileName;
  return outPath.string();
}

std::string getYAxisName(const PlotType type) {
  std::string plotName;
  if (type == PlotType::Both) {
    plotName = "Coverage (%)";
  } else if (type == PlotType::BrCov) {
    plotName = "Branch Coverage (%)";
  } else if (type == PlotType::LineCov) {
    plotName = "Line Coverage (%)";
  }
  return plotName;
}

}  // namespace

LinesBarPlot::LinesBarPlot(const std::string& logDir,
                           const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {
  plt::figure_size(780, 640);
}
LinesBarPlot::~LinesBarPlot() {
  save();
}

void LinesBarPlot::log(size_t oldLines, size_t newLines) {
  if (_current == nullptr) {
    return;
  }

  _current->oldLines = oldLines;
  _current->newLines.emplace_back(newLines);
}

void LinesBarPlot::next(const std::string& label) {
  _barData.emplace_back(LinesBarPlotData{});
  _current = &_barData.back();

  _current->label = label;
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

  std::vector<std::string> testCases;
  std::transform(_barData.cbegin(), _barData.cend(),
                 std::back_inserter(testCases),
                 [](const auto& entry) { return entry.label; });
  size_t n = testCases.size();

  // Hacky way: simulate vertical by inserting \n
  for (auto& l : testCases) {
    for (auto& ch : l) {
      ch = (ch == '_') ? '\n' : ch;
    }
  }

  // Bar values
  std::vector<double> oldLines;
  std::vector<double> newLines;

  std::transform(_barData.cbegin(), _barData.cend(),
                 std::back_inserter(oldLines),
                 [](const auto& entry) { return entry.oldLines; });
  std::transform(
      _barData.cbegin(), _barData.cend(), std::back_inserter(newLines),
      [](const auto& entry) { return compute_average(entry.newLines); });

  // X positions (base for each group)
  std::vector<double> x(n);
  for (size_t i = 0; i < n; ++i)
    x[i] = static_cast<double>(i);

  // Group offset (for 2 bars per group)
  std::vector<double> x1(n), x2(n);
  double width = 0.2;

  for (size_t i = 0; i < n; ++i) {
    x1[i] = x[i] - width;
    x2[i] = x[i];
  }

  plt::bar(x1, oldLines, "black", "-", 0.5, width,
           {{"label", "Original Test Script"}});
  plt::bar(x2, newLines, "black", "-", 0.5, width,
           {{"label", "New Test Script"}});

  // Set ticks and labels
  plt::xticks(x, testCases);
  plt::ylabel("Instructions Count");

  plt::legend();

  plt::pause(0.01);
}

namespace cfg_coverage {

MathplotLogger::MathplotLogger(const std::string& logDir,
                               const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

void MathplotLogger::log(size_t index, const Coverage& coverage) const {
  index_.push_back(static_cast<double>(index));
  _percents.push_back(coverage.getPercentage());

  plot();
}

void MathplotLogger::plot() const {
  plt::clf();                    // Clear previous frame
  plt::plot(index_, _percents);  // Plot updated points
  plt::xlabel("Iteration");
  plt::ylabel("Basic Block Coverage (%)");
  plt::title(" ");
  plt::grid(true);
  plt::pause(0.01);  // Allow time for GUI to update
}

MathplotLogger::~MathplotLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();
}

}  // namespace cfg_coverage

namespace gcov_coverage {

CoverageBoxPlot::CoverageBoxPlot(const std::string& logDir,
                                 const std::string& logFileName,
                                 PlotType type)
    : _type(type), m_path(ensurePath(logDir, logFileName)) {
  plt::figure_size(1120, 640);
}
CoverageBoxPlot::~CoverageBoxPlot() {
  save();
}

void CoverageBoxPlot::next(const std::string& label) {
  _boxData.emplace_back(BoxPlotData{});
  _current = &_boxData.back();

  _current->label = label;
}

void CoverageBoxPlot::log(size_t, const RootReport& coverage) const {
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

  const auto printStats = [](std::vector<double>& data, double x) {
    BoxStats stats = compute_box(data);

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
  };

  int idx = 1;
  const double offsetBase = 0.25;
  for (auto& plotData : plotDatas) {
    printStats(plotData, idx + offsetBase);
    std::cout << labels[idx - 1] << " ";
    std::for_each(plotData.cbegin(), plotData.cend(),
                  [](auto val) { std::cout << val << " "; });
    std::cout << std::endl;
    idx++;
  }

  plt::boxplot(plotDatas, labels, false, {{"patch_artist", "True"}});
  plt::xticks(xticks, labels);

  plt::ylabel(getYAxisName(_type));
  plt::ylim(0.0, 100.0);

  plt::pause(0.01);  // Allow time for GUI to update
}

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

void CoverageBarPlot::log(size_t, const RootReport& coverage) const {
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

MathplotLogger::MathplotLogger(const std::string& logDir,
                               const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}
MathplotLogger::~MathplotLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
}

void MathplotLogger::log(size_t index, const RootReport& coverage) const {
  i_.push_back(static_cast<double>(index));
  lcov_.push_back(coverage.report.lineCov.percent);
  bcov_.push_back(coverage.report.branchCov.percent);
  fcov_.push_back(coverage.report.funcCov.percent);

  plot();
}

void MathplotLogger::plot() const {
  plt::clf();  // Clear previous frame

  // Plot each coverage vector with labels
  plt::plot(i_, lcov_,
            std::map<std::string, std::string>{{"label", "Line Coverage"}});
  plt::plot(i_, bcov_,
            std::map<std::string, std::string>{{"label", "Branch Coverage"}});
  plt::plot(i_, fcov_,
            std::map<std::string, std::string>{{"label", "Function Coverage"}});

  plt::xlabel("Iteration");
  plt::ylabel("Coverage (%)");
  plt::title(" ");
  plt::grid(true);
  plt::legend();     // Show legend with labels
  plt::pause(0.01);  // Allow time for GUI to update
}

StepperComparativeLogger::StepperComparativeLogger(
    const std::string& logDir,
    const std::string& logFileName,
    PlotType type)
    : _type(type), m_path(ensurePath(logDir, logFileName)) {}

StepperComparativeLogger::~StepperComparativeLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();
}

void StepperComparativeLogger::next(const std::string& name) {
  _graphs.emplace_back(Points{});
  _current = &_graphs.back();

  _current->name = name;
  _current->color = ColorCodes[_color++];
  _current->lineStyle = LineStyles[_style++];

  if (_color >= 4)
    _color = 0;
  if (_style >= 7)
    _style = 0;
}

void StepperComparativeLogger::log(size_t index,
                                   const RootReport& coverage) const {
  if (_current == nullptr) {
    return;
  }

  _current->instructions.push_back(static_cast<double>(index));
  _current->lineCov.push_back(coverage.report.lineCov.percent);
  _current->brCov.push_back(coverage.report.branchCov.percent);

  plot();
}

void StepperComparativeLogger::plot() const {
  if (_graphs.empty()) {
    return;
  }

  if ((_updateCounter++ % 40) != 0) {
    return;
  }

  plt::clf();  // Clear previous frame

  for (const auto& graph : _graphs) {
    if (_type == PlotType::BrCov || _type == PlotType::Both) {
      std::string prefix;
      if (_type == PlotType::Both) {
        prefix = "BrCov ";
      }
      plt::plot(graph.instructions, graph.brCov,
                std::map<std::string, std::string>{
                    {"label", prefix + graph.name.c_str()},
                    {"color", graph.color},
                    {"linestyle", graph.lineStyle},
                    {"linewidth", "1.0"}});
    }

    if (_type == PlotType::LineCov || _type == PlotType::Both) {
      std::string prefix;
      if (_type == PlotType::Both) {
        prefix = "LineCov ";
      }

      plt::plot(graph.instructions, graph.lineCov,
                std::map<std::string, std::string>{
                    {"label", prefix + graph.name.c_str()},
                    {"color", graph.color},
                    {"linestyle", graph.lineStyle},
                    {"linewidth", "1.0"}});
    }
  }

  plt::xlabel("Instructions count");
  plt::ylabel(getYAxisName(_type));
  plt::title(" ");
  plt::grid(true);
  plt::legend();     // Show legend with labels
  plt::pause(0.01);  // Allow time for GUI to update
}

}  // namespace gcov_coverage

namespace metasearch {

MathplotLogger::MathplotLogger(const std::string& logDir,
                               const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

void MathplotLogger::log(size_t index, const Solution& solution) const {
  x_.push_back(static_cast<double>(index));
  y_.push_back(solution.objVal);

  plot();
}

void MathplotLogger::plot() const {
  plt::clf();         // Clear previous frame
  plt::plot(x_, y_);  // Plot updated points
  plt::xlabel("Iteration");
  plt::ylabel("Basic Block Coverage (%) ");
  plt::title(" ");
  plt::grid(true);
  plt::pause(0.01);  // Allow time for GUI to update
}

MathplotLogger::~MathplotLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();
}

}  // namespace metasearch

namespace qleaning {

MathplotLogger::MathplotLogger(const std::string& logDir,
                               const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {
  plt::figure_size(640, 640);
}

void MathplotLogger::logReward(size_t episode, const double totalReward) const {
  ieps_.push_back(static_cast<double>(episode));
  rwrd_.push_back(totalReward);

  plot();
}

void MathplotLogger::logLoss(size_t episode, const double averageLoss) const {
  jeps_.push_back(static_cast<double>(episode));
  loss_.push_back(averageLoss);

  plot();
}

void MathplotLogger::plot() const {
  plt::clf();

  plt::subplot2grid(2, 1, 0, 0);
  plt::plot(ieps_, rwrd_,
            std::map<std::string, std::string>{{"label", "Total Reward"},
                                               {"color", "red"},
                                               {"linestyle", "-"},
                                               {"linewidth", "0.5"}});
  plt::grid(true);
  plt::legend();

  plt::subplot2grid(2, 1, 1, 0);
  plt::plot(jeps_, loss_,
            std::map<std::string, std::string>{{"label", "Average Loss"},
                                               {"color", "blue"},
                                               {"linestyle", "-"},
                                               {"linewidth", "0.5"}});
  plt::xlabel("Epochs");
  plt::legend();
  plt::grid(true);

  plt::pause(0.01);
}

MathplotLogger::~MathplotLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();
}

}  // namespace qleaning

}  // namespace cider
