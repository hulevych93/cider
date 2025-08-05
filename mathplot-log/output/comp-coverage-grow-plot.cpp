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

void printStats(const Stats& stats, const std::string& name = "Stats") {
  std::cout << "--- " << name << " ---\n";

  auto printVector = [](const std::string& label,
                        const std::vector<double>& vec) {
    std::cout << label << " (" << vec.size() << "): ";
    for (double val : vec)
      std::cout << val << " ";
    std::cout << "\n";
  };

  printVector("Instructions", stats.instructions);
  printVector("Mean Line Coverage", stats.meanLineCov);
  printVector("Mean Branch Coverage", stats.meanBrCov);
  printVector("Std Line Coverage", stats.stdLineCov);
  printVector("Std Branch Coverage", stats.stdBrCov);
}

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

bool serialize(const Points& obj, serialization::Serializer& serializer) {
  serializer << obj.instructions;
  serializer << obj.lineCov;
  serializer << obj.brCov;
  return true;
}

bool deserialize(Points& obj, const serialization::Deserializer& deserializer) {
  deserializer >> obj.instructions;
  deserializer >> obj.lineCov;
  deserializer >> obj.brCov;
  return true;
}

StepperComparativeLogger::StepperComparativeLogger(
    const std::string& logDir,
    const std::string& logFileName,
    PlotType type)
    : _type(type), m_path(ensurePath(logDir, logFileName)) {}

StepperComparativeLogger::~StepperComparativeLogger() {
  serialize(ensureBinExtension(m_path));
  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();
}

void StepperComparativeLogger::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << _type;
    serializer << _graphs;
    serializer << _order;
    serializer.save(filePath);
  } catch (...) {
    std::cout << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool StepperComparativeLogger::load() {
  try {
    serialization::Deserializer deserializer(ensureBinExtension(m_path));
    deserializer >> _type;
    deserializer >> _graphs;
    deserializer >> _order;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
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
    serialize(ensureBinExtension(m_path));
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

  static int pp = 1;
  if ((pp++ % 100) == 0) {
    plot();
  }
}

void StepperComparativeLogger::plot() const {
  if (_graphs.empty()) {
    return;
  }

  plt::clf();  // Clear previous frame

  int color = 0;
  int marker = 0;

  for (const auto& name : _order) {
    std::string linestyle = "-";
    std::string linestyleWidth = "1.2";

    Stats stats;
    if (name != "Original") {
      stats = getStats(_graphs[name]);

      if (name == "RAND") {
        printStats(stats);
      }
    } else {
      linestyle = "--";
      const auto& points = _graphs[name][0];
      stats.instructions = points.instructions;
      stats.meanLineCov = points.lineCov;
      stats.meanBrCov = points.brCov;

      // === Plot horizontal line for max original coverage ===
      double maxOriginalCoverage =
          *std::max_element(points.brCov.begin(), points.brCov.end());
      std::cout << maxOriginalCoverage << std::endl;

      plt::plot(std::vector<double>{0, stats.instructions.back()},
                std::vector<double>{maxOriginalCoverage, maxOriginalCoverage},
                {{"linestyle", "-."},
                 {"color", "purple"},
                 {"linewidth", "1.0"},
                 {"label", "Original TC max"}});
    }

    if (_type == PlotType::BrCov || _type == PlotType::Both) {
      std::string prefix;
      if (_type == PlotType::Both) {
        prefix = "BrCov ";
      }

      if (name != "Original") {
        std::vector<double> yerr(stats.meanBrCov.size());
        for (size_t i = 0; i < stats.meanBrCov.size(); ++i) {
          yerr[i] = stats.stdBrCov[i];  // or scaled: 0.3 * stats.stdBrCov[i]

          if (name != "RAND") {
            yerr[i] = stats.stdBrCov[i];
          }
        }

        if (false) {
          // Use error bars instead of shaded area
          plt::errorbar(stats.instructions, stats.meanBrCov, yerr,
                        std::map<std::string, std::string>{
                            {"label", prefix + name.c_str()},
                            {"color", ColorCodes[color]},
                            {"linestyle", linestyle.c_str()},
                            {"linewidth", linestyleWidth},
                            {"marker", MarkerStyles[marker]},
                            {"markersize", (marker == 3 ? "2.5" : "1.5")},
                            {"capsize", "2"},
                            {"elinewidth", "1"}});
        } else {
          plt::plot(stats.instructions, stats.meanBrCov,
                    std::map<std::string, std::string>{
                        {"label", prefix + name.c_str()},
                        {"color", ColorCodes[color]},
                        {"linestyle", linestyle.c_str()},
                        {"linewidth", linestyleWidth},
                        {"marker", MarkerStyles[marker]},
                        {"markersize", (marker == 3 ? "2.5" : "1.5")}});
        }
      }
    }

    color++;
    if (color >= 4)
      color = 0;

    marker++;
    if (marker >= 4)
      marker = 0;
  }

  applyPublicationStyle();

  plt::xlabel("Instruction");
  plt::ylabel(getYAxisName(_type));
  plt::title(" ");
  plt::grid(true);
  plt::ylim(0.0, 35.0);
  plt::xlim(0.0, 450.0);

  plt::legend();     // Show legend with labels
  plt::pause(0.01);  // Allow time for GUI to update
}

}  // namespace mathplot
}  // namespace cider
