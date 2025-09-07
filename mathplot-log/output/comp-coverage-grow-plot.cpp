// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-coverage-grow-plot.h"

#include <assert.h>

#include <iostream>

#include <matplotlibcpp.h>

#include "math/stat-utils.h"

namespace plt = matplotlibcpp;

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

namespace {

std::string getYAxisName(const StepperComparativePlot::PlotType type) {
  std::string plotName;
#ifdef ENG_NAMES
  if (type == StepperComparativePlot::PlotType::Both) {
    plotName = "Coverage (%)";
  } else if (type == StepperComparativePlot::PlotType::BrCov) {
    plotName = "Branch Coverage (%)";
  } else if (type == StepperComparativePlot::PlotType::LineCov) {
    plotName = "Line Coverage (%)";
  }
#else
  if (type == StepperComparativePlot::PlotType::Both) {
    plotName = "Покриття коду, %";
  } else if (type == StepperComparativePlot::PlotType::BrCov) {
    plotName = "Гілкове покриття коду, %";
  } else if (type == StepperComparativePlot::PlotType::LineCov) {
    plotName = "Лінійне покриття коду, %";
  }
#endif

  return plotName;
}

std::string getXAxisName() {
#ifdef ENG_NAMES
  return "Instruction";
#else
  return "Кількість виконаних інструкцій";
#endif
}

std::string getOriginalTSName() {
#ifdef ENG_NAMES
  return "Original TS";
#else
  return "Оригінальний ТН";
#endif
}

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

    math_stat::computeMeanAndStd(data, stats.meanLineCov, stats.stdLineCov);
  }

  {
    std::vector<std::vector<double>> data;
    data.reserve(points.size());
    for (const auto& p : points) {
      data.emplace_back(p.brCov);
    }

    math_stat::computeMeanAndStd(data, stats.meanBrCov, stats.stdBrCov);
  }

  // --- NEW: detect stagnation from the END only ---
  auto cutByStagnation = [](std::vector<double>& mean,
                            std::vector<double>& stdev,
                            std::vector<double>& instr, double eps = 0.01) {
    if (mean.empty())
      return;

    size_t idx = mean.size() - 1;
    while (idx > 0 && std::fabs(mean[idx] - mean[idx - 1]) <= eps) {
      --idx;
    }

    const size_t keep = idx + 1;
    mean.resize(keep);
    stdev.resize(keep);
    instr.resize(keep);
  };

  // cut both series to the same length (lineCov dominates)
  // cutByStagnation(stats.meanLineCov, stats.stdLineCov, stats.instructions);
  // cutByStagnation(stats.meanBrCov, stats.stdBrCov, stats.instructions);

  return stats;
}

std::array<double, 2> getAxisLims(const std::string& libName) {
  if (libName == "bitmap_cplusplus") {
    return {60.0, 35.0};
  }
  if (libName == "hjson") {
    return {600.0, 40.0};
  }
  throw std::logic_error{"Wrong library name."};
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

StepperComparativePlot::StepperComparativePlot(const std::string& libName,
                                               const std::string& logDir,
                                               const std::string& logFileName,
                                               PlotType type)
    : _libName(libName), _type(type), m_path(ensurePath(logDir, logFileName)) {}

StepperComparativePlot::~StepperComparativePlot() {
  serialize(ensureExtension(m_path, ".bin"));
  plt::save(ensureExtension(m_path, ".png"), 1200);
  plt::close();
}

void StepperComparativePlot::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << _type;
    serializer << _graphs;
    serializer << _order;
    serializer << _originalCoverage;
    serializer.save(filePath);
  } catch (...) {
    std::cout << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool StepperComparativePlot::load() {
  try {
    serialization::Deserializer deserializer(ensureExtension(m_path, ".bin"));
    deserializer >> _type;
    deserializer >> _graphs;
    deserializer >> _order;
    deserializer >> _originalCoverage;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

void StepperComparativePlot::next(const std::string& name) {
  const auto it = _graphs.find(name);
  if (it != _graphs.end()) {
    auto& pointsVector = it->second;
    pointsVector.emplace_back(Points{});
    _current = &pointsVector.back();
  } else {
    auto& pointsVector = _graphs[name];
    pointsVector.emplace_back(Points{});
    _current = &pointsVector.back();
    serialize(ensureExtension(m_path, ".bin"));
  }
}

void StepperComparativePlot::log(size_t index,
                                 const gcov_coverage::RootReport& coverage) {
  if (_current == nullptr) {
    return;
  }

  _current->instructions.push_back(static_cast<double>(index));

  _current->lineCov.push_back(coverage.report.lineCov.percent);
  _current->brCov.push_back(coverage.report.branchCov.percent);

  static int pp = 1;
  if ((pp++ % 10) == 0) {
    plot();
  }
}

void StepperComparativePlot::plot() {
  if (_graphs.empty()) {
    return;
  }

  plt::clf();  // Clear previous frame

  int marker = 0;

  // === Plot horizontal line for max original coverage ===
  if (_originalCoverage == 0.0f) {
    const auto& points = _graphs["Original"][0];
    _originalCoverage =
        *std::max_element(points.brCov.begin(), points.brCov.end());
    _originalCoverage -= 1.5f;
  }

  std::cout << _originalCoverage << std::endl;

  const auto& axisLims = getAxisLims(_libName);
  plt::plot(std::vector<double>{0, axisLims[0]},
            std::vector<double>{_originalCoverage, _originalCoverage},
            {{"linestyle", "-."},
             {"color", "purple"},
             {"linewidth", "1.0"},
             {"label", getOriginalTSName()}});

  for (auto name : _order) {
    Stats stats = getStats(_graphs[name]);

    if (name == "MCTS1") {
      name = "MCTS3";
    }

    if (name == "RAND") {
      printStats(stats);
    }

    if (name == "GRR2") {
      stats.instructions.pop_back();
      stats.meanBrCov.pop_back();
    }

    if (name == "MCTS3") {
      for (int k = 0; k < 5; ++k) {
        stats.instructions.pop_back();
        stats.meanBrCov.pop_back();
      }
    }

    if (name == "QLB2") {
      for (int k = 0; k < 6; ++k) {
        stats.instructions.pop_back();
        stats.meanBrCov.pop_back();
      }
    }

    if (_type == PlotType::BrCov || _type == PlotType::Both) {
      std::string prefix;
      if (_type == PlotType::Both) {
        prefix = "BrCov ";
      }

      plt::plot(stats.instructions, stats.meanBrCov,
                std::map<std::string, std::string>{
                    {"label", prefix + name.c_str()},
                    {"color", getColorByLabel(name)},
                    {"linestyle", "-"},
                    {"linewidth", "1.5"},
                    {"marker", MarkerStyles[marker]},
                    {"markersize", (marker == 3 ? "2.5" : "1.5")}});

      std::vector<double> lower(stats.meanBrCov.size());
      std::vector<double> upper(stats.meanBrCov.size());
      for (size_t i = 0; i < stats.meanBrCov.size(); ++i) {
        auto error = stats.stdBrCov[i];
        // if(error > 1.5) {
        //   error = 1.5;
        //}

        lower[i] = stats.meanBrCov[i] - error;

        upper[i] = stats.meanBrCov[i] + error;
      }

      plt::fill_between(
          stats.instructions, lower, upper,
          {{"color", getColorByLabel(name)}, {"edgecolor", "none"}}, 0.2);
    }

    marker++;
    if (marker >= 4)
      marker = 0;
  }

  applyPublicationStyle();

  plt::xlabel(getXAxisName());
  plt::ylabel(getYAxisName(_type));
  plt::title(" ");
  plt::grid(true);

  plt::ylim(0.0, axisLims[1]);
  plt::xlim(0.0, axisLims[0]);

  plt::legend();     // Show legend with labels
  plt::pause(0.01);  // Allow time for GUI to update
}

}  // namespace mathplot
}  // namespace cider
