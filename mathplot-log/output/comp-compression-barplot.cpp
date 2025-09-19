// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-compression-barplot.h"

#include <assert.h>

#include <iostream>

#include "math/stat-utils.h"
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
  return "Коефіцієнт стиснення, ум. од.";
#endif
}

static std::string getXAxisName() {
#ifdef ENG_NAMES
#else
  return "Метод / конфігурація";
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
    serializer << _order;
    serializer.save(filePath);
  } catch (...) {
    std::cout << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool CompressionBarPlot::load() {
  try {
    serialization::Deserializer deserializer(ensureExtension(m_path, ".bin"));
    deserializer >> _barData;
    deserializer >> _order;
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

  std::vector<std::string> methods;

  int i = 0;

  std::vector<double> xg(_barData.size());

  for (const auto& orderName : _order) {
    const auto it = _barData.find(orderName);
    if (it == _barData.end()) {
      std::cout << "Warning method not simulated: " << orderName << std::endl;
      continue;
    }

    std::vector<double> means;
    std::vector<double> stddevs;

    std::vector<double> x(1);
    x[0] = static_cast<double>(i);
    xg[i] = static_cast<double>(i);
    i++;

    methods.push_back(it->first);

    const auto mean = math_stat::mean(it->second);
    means.emplace_back(mean);
    stddevs.emplace_back(math_stat::stddev(it->second, mean));

    plt::bar(x, means, "black", "-", 1.0, 0.8,
             {{"color", getColorByLabel(it->first)}});
    plt::errorbar(x, means, stddevs,
                  {{"fmt", "none"}, {"ecolor", "red"}, {"capsize", "3"}});
  }

  if (_barData.size() > 4) {
    makeLegentByGroups(getColorGroups(), {0.54, 1.0});
    rotateXTicks90();
  }

  plt::xticks(xg, methods, {{"fontsize", "7"}});
  plt::ylabel(getYAxisName());
  plt::xlabel(getXAxisName());

  applyPublicationStyle();

  plt::save(ensureExtension(m_path, ".eps"), 1200);
  plt::close();

  serialize(ensureExtension(m_path, ".bin"));
}

}  // namespace mathplot
}  // namespace cider
