// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-lines-barplot.h"

#include <assert.h>

#include <iostream>

#include "mathplot-log/utils.h"

#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

namespace cider {
namespace mathplot {

LinesBarPlot::LinesBarPlot(const std::string& logDir,
                           const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

LinesBarPlot::~LinesBarPlot() {}

void LinesBarPlot::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << _barData;
    serializer << _oldLines;
    serializer.save(filePath);
  } catch (...) {
    std::cout << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool LinesBarPlot::load() {
  try {
    serialization::Deserializer deserializer(ensureBinExtension(m_path));
    deserializer >> _barData;
    deserializer >> _oldLines;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

void LinesBarPlot::log(const std::string& label,
                       const std::string& method,
                       size_t oldLines,
                       size_t newLines) {
  const auto it = _barData.find(label);
  if (it != _barData.end()) {
    auto& labelMap = it->second;
    auto& data = labelMap[method];
    data.emplace_back(newLines);
  } else {
    std::cout << label << std::endl;
    auto& labelMap = _barData[label];
    auto& data = labelMap[method];
    _oldLines[label] = oldLines;
    data.emplace_back(newLines);
  }
}

std::vector<std::vector<double>> convertToMethodMajor(
    const std::vector<std::vector<double>>& lines) {
  if (lines.empty())
    return {};

  size_t numTestCases = lines.size();
  size_t numMethods = lines[0].size();

  std::vector<std::vector<double>> linesByMethod(
      numMethods, std::vector<double>(numTestCases, 0.0));

  for (size_t i = 0; i < numTestCases; ++i) {
    for (size_t j = 0; j < numMethods; ++j) {
      linesByMethod[j][i] = lines[i][j];
    }
  }

  return linesByMethod;
}

void LinesBarPlot::plot() {
  static bool done = false;
  if (!done) {
    done = true;
  } else {
    return;
  }
  plt::clf();

  std::vector<std::string> testCases;

  // Bar values
  std::vector<std::vector<double>> lines;
  std::vector<std::string> names;
  std::unordered_set<std::string> namesDouble;

  names.emplace_back("Original");

  int i = 1;
  for (const auto& labelIter : _barData) {
    testCases.emplace_back(std::to_string(i));

    lines.emplace_back(std::vector<double>{});
    lines[i - 1].emplace_back(_oldLines[labelIter.first]);

    int j = 1;
    auto& methodData = labelIter.second;
    for (const auto& methodIt : methodData) {
      if (namesDouble.find(methodIt.first) == namesDouble.cend()) {
        namesDouble.emplace(methodIt.first);
        names.emplace_back(methodIt.first);
      }

      lines[i - 1].emplace_back(compute_average(methodIt.second));
      j++;
    }
    i++;
  }

  auto methodsAndLines = convertToMethodMajor(lines);

  assert(names.size() <= 3);

  size_t n = testCases.size();

  // X positions (base for each group)
  std::vector<double> x(n);
  for (size_t i = 1; i < n; ++i)
    x[i] = static_cast<double>(i);

  // Group offset (for 2 bars per group)
  std::vector<double> xg[names.size()];
  double width = 0.3;

  for (int method = 0; method < names.size(); ++method) {
    xg[method] = std::vector<double>(n, 0.0f);
    for (int TC = 0; TC < n; ++TC) {
      xg[method][TC] = x[TC] + width * method;
    }
  }

  for (int method = 0; method < names.size(); ++method) {
    plt::bar(xg[method], methodsAndLines[method], "black", "-", 0.5, width,
             {{"label", names[method]}});
  }

  // Set ticks and labels
  plt::xticks(x, testCases, {{"fontsize", "5"}});
  plt::ylabel("Instructions Count");
  plt::xlabel("Test Case");

  plt::legend();

  applyPublicationStyle();
  plt::pause(5.5);

  serialize(ensureBinExtension(m_path));
  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();
}

}  // namespace mathplot
}  // namespace cider
