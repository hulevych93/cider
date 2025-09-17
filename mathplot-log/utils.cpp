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

#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;

namespace cider {

std::string ensureExtension(const std::string& path, const std::string& ext) {
  std::filesystem::path filePath(path);

  // Check if the extension is already .png (case insensitive)
  if (filePath.extension() == ext) {
    return filePath.string();
  }

  // Add .png extension
  filePath.replace_extension(ext);
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

std::string ensureCsvExtension(const std::string& path) {
  std::filesystem::path filePath(path);

  // Check if the extension is already .png (case insensitive)
  if (filePath.extension() == ".csv") {
    return filePath.string();
  }

  // Add .png extension
  filePath.replace_extension(".csv");
  return filePath.string();
}

std::string ensurePath(const std::string& logDir,
                       const std::string& logFileName) {
  std::filesystem::path outPath(logDir);
  std::filesystem::create_directories(outPath);
  outPath /= logFileName;
  return outPath.string();
}

double kruskal_wallis(const std::vector<std::vector<double>>& groups) {
  try {
    py::module_ stats = py::module_::import("scipy.stats");

    // формуємо список Python-списків
    py::list py_groups;
    for (const auto& g : groups) {
      py::list py_group;
      for (double val : g) {
        py_group.append(val);
      }
      py_groups.append(py_group);
    }

    // викликаємо scipy.stats.kruskal(*groups)
    py::object result = stats.attr("kruskal")(*py_groups);

    return result.attr("pvalue").cast<double>();
  } catch (const py::error_already_set& e) {
    std::cerr << "Kruskal–Wallis error: " << e.what() << std::endl;
    return -1.0;
  }
}

double mann_whitney_u(const std::vector<double>& group1,
                      const std::vector<double>& group2,
                      const std::string& alternative) {
  try {
    py::module_ stats = py::module_::import("scipy.stats");

    py::list py_group1;
    for (double val : group1)
      py_group1.append(val);

    std::cout << "py_group1: " << group1.size() << std::endl;
    std::cout << "py_group2: " << group2.size() << std::endl;

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

void setAxisPolicy() {
  try {
    py::module_ plt = py::module_::import("matplotlib.pyplot");
    py::module_ ticker = py::module_::import("matplotlib.ticker");
    // === New: set major tick frequency ===
    py::object MultipleLocator = ticker.attr("MultipleLocator");

    py::object ax = plt.attr("gca")();

    // Y axis: major ticks every 5
    ax.attr("yaxis").attr("set_major_locator")(MultipleLocator(10));

    // Optional: add minor ticks
    ax.attr("xaxis").attr("set_major_locator")(MultipleLocator(4));

  } catch (const std::exception& e) {
    std::cerr << "Style error: " << e.what() << std::endl;
  }
}

void tightLighout() {
  try {
    py::module_ plt = py::module_::import("matplotlib.pyplot");
    plt.attr("tight_layout")();
    plt.attr("subplots_adjust")("left"_a = 0.15);
    plt.attr("subplots_adjust")("bottom"_a = 0.15);

  } catch (const std::exception& e) {
    std::cerr << "Rotate error: " << e.what() << std::endl;
  }
}

void rotateXTicks90() {
  try {
    py::module_ plt = py::module_::import("matplotlib.pyplot");
    py::object ax = plt.attr("gca")();  // get current axes

    ax.attr("tick_params")("axis"_a = "x", "labelrotation"_a = 90);

    // піджати графік, звільнити місце під підписи
    plt.attr("tight_layout")();

    // або ж вручну налаштувати поля
    plt.attr("subplots_adjust")("bottom"_a = 0.22);  // 25% поля знизу

  } catch (const std::exception& e) {
    std::cerr << "Rotate error: " << e.what() << std::endl;
  }
}

void makeLegentByGroups(const std::vector<std::string>& groups,
                        const std::vector<double>& positions) {
  std::vector<std::string> legendLabels = groups;
  std::vector<std::string> legendColors;

  for (const auto& group : legendLabels) {
    legendColors.push_back(getColorByLabel(group));
  }

  for (size_t j = 0; j < legendLabels.size(); ++j) {
    plt::bar(std::vector<double>{1}, std::vector<double>{0}, "black", "-", 0.5,
             0.8, {{"color", legendColors[j]}, {"label", legendLabels[j]}});
  }

  plt::legend({{"fontsize", "8"}, {"loc", "upper center"}}, positions);
}

void disableFrame() {
  try {
    py::module_ plt = py::module_::import("matplotlib.pyplot");

    py::object ax = plt.attr("gca")();

    ax.attr("spines")["top"].attr("set_visible")(false);
    ax.attr("spines")["right"].attr("set_visible")(false);
    ax.attr("spines")["bottom"].attr("set_visible")(false);
    ax.attr("spines")["left"].attr("set_visible")(false);

    ax.attr("get_xaxis")().attr("set_visible")(false);
    ax.attr("get_yaxis")().attr("set_visible")(false);

  } catch (const std::exception& e) {
    std::cerr << "disableFrame error: " << e.what() << std::endl;
  }
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
