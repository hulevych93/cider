// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-time-barplot.h"

#include <assert.h>
#include <tlog.h>

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
  return "Average TS formation time, ms";
#else
  return "Середній час виконання, мкс";
#endif
}

static std::string getXAxisName() {
#ifdef ENG_NAMES
  return "Configuration";
#else
  return "Метод / конфігурація";
#endif
}

}  // namespace

TimesBarPlot::TimesBarPlot(const std::string& logDir,
                           const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

TimesBarPlot::~TimesBarPlot() {}

void TimesBarPlot::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << _barData;
    serializer << _order;
    serializer.save(filePath);
  } catch (...) {
    tlog_info << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool TimesBarPlot::load() {
  try {
    serialization::Deserializer deserializer(ensureExtension(m_path, ".bin"));
    deserializer >> _barData;
    deserializer >> _order;
  } catch (const std::exception& e) {
    tlog_info << e.what() << std::endl;
    return false;
  }
  return true;
}

void TimesBarPlot::log(const std::string& method, size_t time) {
  _barData[method].emplace_back(time);
}

void TimesBarPlot::plot() {
  plt::clf();

  std::vector<std::string> methods;
  std::vector<double> allMeans;
  std::vector<double> allStddevs;

  // ===== FIRST PASS: compute mean and std =====
  for (const auto& orderName : _order) {
    const auto it = _barData.find(orderName);
    if (it == _barData.end()) {
      tlog_info << "Warning method not simulated: " << orderName << std::endl;
      continue;
    }

    methods.push_back(it->first);

    const double mean = math_stat::mean(it->second);
    const double stddev = math_stat::stddev(it->second, mean);

    allMeans.emplace_back(mean);
    allStddevs.emplace_back(stddev);
  }

  // ===== compute global max mean =====
  double maxMean = 0.0;
  for (double m : allMeans) {
    if (m > maxMean) {
      maxMean = m;
    }
  }

  if (maxMean == 0.0) {
    std::cout << "Error: maxMean is zero, cannot compute FT." << std::endl;
    return;
  }

  // ===== SECOND PASS: plot bars =====
  int i = 0;
  std::vector<double> xg(methods.size());

  for (size_t idx = 0; idx < methods.size(); ++idx) {
    const std::string& methodName = methods[idx];

    std::vector<double> means(1);
    std::vector<double> stddevs(1);
    std::vector<double> x(1);

    x[0] = static_cast<double>(i);
    xg[i] = static_cast<double>(i);
    i++;

    means[0] = allMeans[idx];
    stddevs[0] = allStddevs[idx] * 0.5;  // як у вас було

    plt::bar(x, means, "black", "-", 1.0, 0.8,
             {{"color", getColorByLabel(methodName)}});

    plt::errorbar(x, means, stddevs,
                  {{"fmt", "none"}, {"ecolor", "red"}, {"capsize", "3"}});
  }

  makeLegentByGroups(getColorGroups(_order), {0.31, 1.0});
  plt::xticks(xg, methods, {{"fontsize", "7"}});
  plt::ylabel(getYAxisName());
  plt::xlabel(getXAxisName());

  applyPublicationStyle();

  if (_barData.size() > 5) {
    rotateXTicks90();
  }

  plt::save(ensureExtension(m_path, ".eps"), 1200);
  plt::close();

  serialize(ensureExtension(m_path, ".bin"));

  // ===== PRINT FT WITH ERROR =====
  std::cout << "\n=== Formation Time Coefficient (FT) ===\n";

  for (size_t k = 0; k < methods.size(); ++k) {
    const double ft = allMeans[k] / maxMean;
    const double ftErr = allStddevs[k] / maxMean;

    std::cout << methods[k] << " | FT = " << ft << " ± " << ftErr << std::endl;
  }
}

}  // namespace mathplot
}  // namespace cider
