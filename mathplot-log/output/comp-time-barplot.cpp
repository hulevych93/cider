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
#else
  return "Середній час виконання, мкс";
#endif
}

static std::string getXAxisName() {
#ifdef ENG_NAMES
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
  const auto it = _barData.find(method);
  if (it != _barData.end()) {
    auto& data = it->second;
    data.emplace_back(time);
  } else {
    auto& data = _barData[method];
    data.emplace_back(time);
  }
}

void TimesBarPlot::plot() {
  plt::clf();

  std::vector<std::string> methods;

  int i = 0;

  std::vector<double> xg(_barData.size());

  for (const auto& orderName : _order) {
    const auto it = _barData.find(orderName);
    if (it == _barData.end()) {
      tlog_info << "Warning method not simulated: " << orderName << std::endl;
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

  makeLegentByGroups(getColorGroups(_order), {0.67, 1.0});

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
}

}  // namespace mathplot
}  // namespace cider
