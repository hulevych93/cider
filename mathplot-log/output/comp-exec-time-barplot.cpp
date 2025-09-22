// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-exec-time-barplot.h"

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
  return "Average execution time, µs";
#else
  return "Середній час виконання ТН, мкс";
#endif
}

static std::string getXAxisName() {
#ifdef ENG_NAMES
  return "Method / configuration";
#else
  return "Метод / конфігурація";
#endif
}

std::string getOriginalTSName() {
#ifdef ENG_NAMES
  return "Original TS";
#else
  return "Оригінальний ТН";
#endif
}

}  // namespace

bool serialize(const ExecTimesData& obj,
               serialization::Serializer& serializer) {
  serializer << obj.oldTimes;
  serializer << obj.newTimes;
  return true;
}

bool deserialize(ExecTimesData& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.oldTimes;
  deserializer >> obj.newTimes;
  return true;
}

ExecTimesBarPlot::ExecTimesBarPlot(const std::string& logDir,
                                   const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

ExecTimesBarPlot::~ExecTimesBarPlot() {}

void ExecTimesBarPlot::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << _barData;
    serializer << _order;
    serializer.save(filePath);
  } catch (...) {
    tlog_info << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool ExecTimesBarPlot::load() {
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

void ExecTimesBarPlot::log(const std::string& method,
                           size_t oldTime,
                           size_t newTime) {
  const auto it = _barData.find(method);
  if (it != _barData.end()) {
    auto& data = it->second;
    data.oldTimes.emplace_back(oldTime);
    data.newTimes.emplace_back(newTime);
  } else {
    auto& data = _barData[method];
    data.oldTimes.emplace_back(oldTime);
    data.newTimes.emplace_back(newTime);
  }
}

void ExecTimesBarPlot::plot() {
  plt::clf();

  std::vector<std::string> methods;
  std::vector<double> xg;

  int i = 0;

  std::vector<size_t> oldTimes;

  for (const auto& orderName : _order) {
    const auto it = _barData.find(orderName);
    if (it == _barData.end()) {
      tlog_info << "Warning method not simulated: " << orderName << std::endl;
      continue;
    }

    const auto& times = it->second;

    if (times.oldTimes.empty() || times.newTimes.empty()) {
      tlog_info << "Method " << orderName
                << " missing old/new times for plotting!" << std::endl;
      continue;
    }

    std::copy(times.oldTimes.cbegin(), times.oldTimes.cend(),
              std::back_inserter(oldTimes));

    double newMean = math_stat::mean(times.newTimes);
    double newStd = math_stat::stddev(times.newTimes, newMean);

    double xpos = static_cast<double>(i);

    std::vector<double> xNew = {xpos};
    std::vector<double> yNew = {newMean};
    plt::bar(xNew, yNew, "black", "-", 1.0, 0.8,
             {{"color", getColorByLabel(orderName)}});
    plt::errorbar(xNew, yNew, {newStd},
                  {{"fmt", "none"}, {"ecolor", "red"}, {"capsize", "3"}});

    methods.push_back(orderName);
    xg.push_back(xpos);
    i++;
  }

  double oldMean = math_stat::mean(oldTimes);
  tlog_info << oldMean << std::endl;

  plt::plot(std::vector<double>{0.0, xg.back()},
            std::vector<double>{oldMean, oldMean},
            {{"linestyle", "-."},
             {"color", "purple"},
             {"linewidth", "1.5"},
             {"label", getOriginalTSName()}});

  makeLegentByGroups(getColorGroups(), {0.84, 0.9});

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
