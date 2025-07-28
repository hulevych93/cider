// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "utils.h"

#include <algorithm>
#include <numeric>
#include <vector>

#include <pybind11/embed.h>
namespace py = pybind11;
using namespace py::literals;

#include <filesystem>
#include <iostream>

namespace cider {

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

void computeMeanAndStd(const std::vector<std::vector<double>>& values,
                       std::vector<double>& meanOut,
                       std::vector<double>& stdOut) {
  size_t maxLen = 0;
  for (const auto& vec : values) {
    maxLen = std::max(maxLen, vec.size());
  }

  meanOut.resize(maxLen, 0.0);
  stdOut.resize(maxLen, 0.0);
  std::vector<size_t> counts(maxLen, 0);

  // Сума для середнього
  for (const auto& vec : values) {
    for (size_t i = 0; i < vec.size(); ++i) {
      meanOut[i] += vec[i];
      counts[i]++;
    }
  }
  for (size_t i = 0; i < maxLen; ++i) {
    if (counts[i] > 0)
      meanOut[i] /= counts[i];
  }

  // Сума квадратів відхилень
  for (const auto& vec : values) {
    for (size_t i = 0; i < vec.size(); ++i) {
      double diff = vec[i] - meanOut[i];
      stdOut[i] += diff * diff;
    }
  }
  for (size_t i = 0; i < maxLen; ++i) {
    if (counts[i] > 1)
      stdOut[i] = std::sqrt(stdOut[i] / (counts[i] - 1));
    else
      stdOut[i] = 0.0;  // немає std для одного значення
  }
}

std::string ensurePngExtension(const std::string& path) {
  std::filesystem::path filePath(path);

  // Check if the extension is already .png (case insensitive)
  if (filePath.extension() == ".eps") {
    return filePath.string();
  }

  // Add .png extension
  filePath.replace_extension(".eps");
  return filePath.string();
}

std::string ensureBinExtension(const std::string& path) {
  std::filesystem::path filePath(path);

  // Check if the extension is already .png (case insensitive)
  if (filePath.extension() == ".bin") {
    return filePath.string();
  }

  // Add .png extension
  filePath.replace_extension(".bin");
  return filePath.string();
}

std::string ensurePath(const std::string& logDir,
                       const std::string& logFileName) {
  std::filesystem::path outPath(logDir);
  std::filesystem::create_directories(outPath);
  outPath /= logFileName;
  return outPath.string();
}

double mann_whitney_u(const std::vector<double>& group1,
                      const std::vector<double>& group2,
                      const std::string& alternative) {
  try {
    py::module_ stats = py::module_::import("scipy.stats");

    py::list py_group1;
    for (double val : group1)
      py_group1.append(val);

    py::list py_group2;
    for (double val : group2)
      py_group2.append(val);

    py::object result = stats.attr("mannwhitneyu")(
        py_group1, py_group2, py::arg("alternative") = alternative);

    return result.attr("pvalue").cast<double>();
  } catch (const py::error_already_set& e) {
    return -1.0;
  }
}

void applyFonts() {
  py::module_ plt = py::module_::import("matplotlib.pyplot");

  py::object rcParams = plt.attr("rcParams");

  rcParams["font.family"] = py::list(py::make_tuple("Helvetica"));
  rcParams["font.size"] = 6.0;
  rcParams["lines.linewidth"] = 0.6;
  rcParams["patch.linewidth"] = 0.6;
}

void applyPublicationStyle() {
  try {
    py::module_ plt = py::module_::import("matplotlib.pyplot");

    py::object ax = plt.attr("gca")();

    ax.attr("grid")(true, "which"_a = "both", "axis"_a = "both");

    ax.attr("set_axisbelow")(true);  // grid below boxes
    py::object gridlines = ax.attr("get_xgridlines")();
    for (auto g : gridlines) {
      g.attr("set_linestyle")("--");
      g.attr("set_linewidth")(0.25);
      g.attr("set_alpha")(0.6);
      g.attr("set_color")("gray");
    }

    gridlines = ax.attr("get_ygridlines")();
    for (auto g : gridlines) {
      g.attr("set_linestyle")("--");
      g.attr("set_linewidth")(0.25);
      g.attr("set_alpha")(0.6);
      g.attr("set_color")("gray");
    }

    ax.attr("tick_params")("direction"_a = "out", "axis"_a = "both");

    ax.attr("spines")["top"].attr("set_visible")(false);
    ax.attr("spines")["right"].attr("set_visible")(false);

  } catch (const std::exception& e) {
    std::cerr << "Style error: " << e.what() << std::endl;
  }
}

}  // namespace cider
