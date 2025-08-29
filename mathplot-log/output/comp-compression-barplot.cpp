// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-compression-barplot.h"

#include <assert.h>

#include <iostream>

#include "mathplot-log/utils.h"

#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

namespace cider {
namespace mathplot {

namespace {

static std::string getYAxisName() {
#ifdef ENG_NAMES
#else
  return "Середній коефіцієнт стиснення тестового сценарію, ум. од.";
#endif
}

static std::string getXAxisName() {
#ifdef ENG_NAMES
#else
  return "Метод";
#endif
}

}  // namespace

CompressionBarPlot::CompressionBarPlot(const std::string& logDir,
                                       const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

CompressionBarPlot::~CompressionBarPlot() {}

void CompressionBarPlot::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << _barData;
    serializer.save(filePath);
  } catch (...) {
    std::cout << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool CompressionBarPlot::load() {
  try {
    serialization::Deserializer deserializer(ensureBinExtension(m_path));
    deserializer >> _barData;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

void CompressionBarPlot::log(const std::string& method, double coefficient) {
  const auto it = _barData.find(method);
  if (it != _barData.end()) {
    auto& data = it->second;
    data.emplace_back(coefficient);
  } else {
    auto& data = _barData[method];
    data.emplace_back(coefficient);
  }
}

void CompressionBarPlot::plot() {
  plt::clf();

  std::vector<std::pair<std::string, double>> methodTimes;
  for (const auto& iter : _barData) {
    methodTimes.emplace_back(iter.first, compute_average(iter.second));
  }

  // відсортуємо методи для стабільності
  std::sort(methodTimes.begin(), methodTimes.end(),
            [](auto& a, auto& b) { return a.first < b.first; });

  std::vector<std::string> methods;
  std::vector<double> times;
  std::vector<double> x(methodTimes.size());

  for (size_t i = 0; i < methodTimes.size(); ++i) {
    methods.push_back(methodTimes[i].first);
    times.push_back(methodTimes[i].second);
    x[i] = static_cast<double>(i);
  }

  plt::bar(x, times, "black", "-", 0.5, 0.8);

  plt::xticks(x, methods, {{"fontsize", "5"}});
  plt::ylabel(getYAxisName());
  plt::xlabel(getXAxisName());

  applyPublicationStyle();

  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();

  serialize(ensureBinExtension(m_path));
}

}  // namespace mathplot
}  // namespace cider
