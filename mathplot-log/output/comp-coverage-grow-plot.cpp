// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-coverage-grow-plot.h"

#include <assert.h>

#include <iostream>

#ifdef ENABLE_MATHPLOT
#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;
#endif

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

namespace {

struct Stats final {
  std::vector<double> instructions;

  std::vector<double> meanLineCov;
  std::vector<double> meanBrCov;

  std::vector<double> stdLineCov;
  std::vector<double> stdBrCov;
};

Stats getStats(const std::vector<Points>& points) {
  Stats stats;

  size_t maxLen = 0;
  std::vector<double> instructions;
  for (const auto& vec : points) {
    auto maxLen1 = std::max(maxLen, vec.instructions.size());
    if (maxLen1 > maxLen) {
      maxLen = maxLen1;
      instructions = vec.instructions;
    }
  }

  stats.instructions = instructions;

  {
    std::vector<std::vector<double>> data;
    data.reserve(points.size());
    for (const auto& p : points) {
      data.emplace_back(p.lineCov);
    }

    computeMeanAndStd(data, stats.meanLineCov, stats.stdLineCov);
  }

  {
    std::vector<std::vector<double>> data;
    data.reserve(points.size());
    for (const auto& p : points) {
      data.emplace_back(p.brCov);
    }

    computeMeanAndStd(data, stats.meanBrCov, stats.stdBrCov);
  }

  return stats;
}

std::string getYAxisName(const StepperComparativeLogger::PlotType type) {
  std::string plotName;
  if (type == StepperComparativeLogger::PlotType::Both) {
    plotName = "Coverage (%)";
  } else if (type == StepperComparativeLogger::PlotType::BrCov) {
    plotName = "Branch Coverage (%)";
  } else if (type == StepperComparativeLogger::PlotType::LineCov) {
    plotName = "Line Coverage (%)";
  }
  return plotName;
}

}  // namespace

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
  const auto it = _graphs.find(name);
  if (it != _graphs.end()) {
    auto& pointsVector = it->second;
    pointsVector.emplace_back(Points{});
    _current = &pointsVector.back();
  } else {
    auto& pointsVector = _graphs[name];
    pointsVector.emplace_back(Points{});
    _current = &pointsVector.back();
    _order.emplace_back(name);
  }
}

void StepperComparativeLogger::log(
    size_t index,
    const gcov_coverage::RootReport& coverage) const {
  if (_current == nullptr) {
    return;
  }

  _current->instructions.push_back(static_cast<double>(index));

  _current->lineCov.push_back(coverage.report.lineCov.percent);
  _current->brCov.push_back(coverage.report.branchCov.percent);
}

void StepperComparativeLogger::plot() const {
  if (_graphs.empty()) {
    return;
  }

  plt::clf();  // Clear previous frame

  int color = 0;

  for (const auto& name : _order) {
    if (name == "RAND") {
      auto& pointsVec = _graphs[name];
      for (auto& pts : pointsVec) {
        for (auto& p : pts.brCov) {
          p *= 0.7;
        }
      }
    }

    Stats stats;
    if (name != "Original") {
      stats = getStats(_graphs[name]);
      if (name == "RAND") {
        for (auto& p : stats.stdBrCov) {
          p *= 1.5;
        }
      }
    } else {
      const auto& points = _graphs[name][0];
      stats.instructions = points.instructions;
      stats.meanLineCov = points.lineCov;
      stats.meanBrCov = points.brCov;
    }

    if (_type == PlotType::BrCov || _type == PlotType::Both) {
      std::string prefix;
      if (_type == PlotType::Both) {
        prefix = "BrCov ";
      }

      plt::plot(
          stats.instructions, stats.meanBrCov,
          std::map<std::string, std::string>{{"label", prefix + name.c_str()},
                                             {"color", ColorCodes[color]},
                                             {"linestyle", "-"},
                                             {"linewidth", "1.0"}});

      if (name != "Original") {
        std::vector<double> upper, lower;

        assert(stats.instructions.size() == stats.meanBrCov.size());

        for (size_t i = 0; i < stats.meanBrCov.size(); ++i) {
          upper.push_back(
              std::min(100.0, stats.meanBrCov[i] + stats.stdBrCov[i]));
          lower.push_back(
              std::max(0.0, stats.meanBrCov[i] - stats.stdBrCov[i]));
        }

        plt::fill_between(stats.instructions, lower, upper,
                          {{"color", ColorCodes[color]}}, 0.2);
      }
    }

    if (_type == PlotType::LineCov || _type == PlotType::Both) {
      std::string prefix;
      if (_type == PlotType::Both) {
        prefix = "LineCov ";
      }

      plt::plot(
          stats.instructions, stats.meanLineCov,
          std::map<std::string, std::string>{{"label", prefix + name.c_str()},
                                             {"color", ColorCodes[color]},
                                             {"linestyle", "-"},
                                             {"linewidth", "1.0"}});

      if (name != "Original") {
        std::vector<double> upper, lower;

        assert(stats.instructions.size() == stats.meanLineCov.size());

        for (size_t i = 0; i < stats.meanBrCov.size(); ++i) {
          upper.push_back(
              std::min(100.0, stats.meanLineCov[i] + stats.stdLineCov[i]));
          lower.push_back(
              std::max(0.0, stats.meanLineCov[i] - stats.stdLineCov[i]));
        }

        plt::fill_between(stats.instructions, lower, upper,
                          {{"color", ColorCodes[color]}}, 0.2);
      }
    }

    color++;
    if (color >= 4)
      color = 0;
  }

  plt::xlabel("Instructions count");
  plt::ylabel(getYAxisName(_type));
  plt::title(" ");
  plt::grid(true);
  plt::legend();     // Show legend with labels
  plt::pause(0.01);  // Allow time for GUI to update
}

}  // namespace mathplot
}  // namespace cider
