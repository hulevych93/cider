// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-exec-time-barplot.h"

#include <assert.h>
#include <tlog.h>

#include "math/stat-utils.h"
#include "mathplot-log/utils.h"

#include <matplotlibcpp.h>
#include <chrono>

namespace plt = matplotlibcpp;

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

namespace cider {
namespace mathplot {

namespace {

static std::string getYAxisName() {
#ifdef ENG_NAMES
  return "Average TS execution time, ms";
#else
  return "Середній час виконання ТН, мс";
#endif
}

static std::string getXAxisName() {
#ifdef ENG_NAMES
  return "Configuration";
#else
  return "Конфігурація";
#endif
}

std::string getOriginalTSName() {
#ifdef ENG_NAMES
  return "Original TS";
#else
  return "Оригінальний ТН";
#endif
}

static void logECRStats(const std::string& method,
                        const std::vector<size_t>& oldTimes,
                        const std::vector<size_t>& newTimes) {
  assert(oldTimes.size() == newTimes.size());
  if (oldTimes.empty())
    return;

  std::vector<double> ecrVals;
  ecrVals.reserve(oldTimes.size());

  for (size_t i = 0; i < oldTimes.size(); ++i) {
    const double tOld = oldTimes[i];
    const double tNew = newTimes[i];
    if (tOld <= 0.0)
      continue;

    // Формула (13)
    ecrVals.emplace_back((tOld - tNew) / tOld);
  }

  if (ecrVals.empty())
    return;

  const double meanECR = math_stat::mean(ecrVals);
  const double stdECR = math_stat::stddev(ecrVals, meanECR);
  const double stderrECR =
      stdECR / std::sqrt(static_cast<double>(ecrVals.size()));

  tlog_info << "[ECR] " << method << " mean=" << meanECR
            << " stderr=" << stderrECR << " n=" << ecrVals.size() << std::endl;
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
  constexpr double kMicroToMilli = 1.0 / 1000.0;

  const double oldTimeMs = static_cast<double>(oldTime) * kMicroToMilli;
  const double newTimeMs = static_cast<double>(newTime) * kMicroToMilli;

  const auto it = _barData.find(method);
  if (it != _barData.end()) {
    auto& data = it->second;
    data.oldTimes.emplace_back(oldTimeMs);
    data.newTimes.emplace_back(newTimeMs);
  } else {
    auto& data = _barData[method];
    data.oldTimes.emplace_back(oldTimeMs);
    data.newTimes.emplace_back(newTimeMs);
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

    logECRStats(orderName, times.oldTimes, times.newTimes);

    std::copy(times.oldTimes.cbegin(), times.oldTimes.cend(),
              std::back_inserter(oldTimes));

    double newMean = math_stat::mean(times.newTimes);
    double newStd = math_stat::stddev(times.newTimes, newMean);

    double xpos = static_cast<double>(i);

    std::vector<double> xNew = {xpos};
    std::vector<double> yNew = {newMean};
    plt::bar(xNew, yNew, "black", "-", 1.0, 0.8,
             {{"color", getColorByLabel(orderName)}});
    plt::errorbar(xNew, yNew, {newStd * 0.7},
                  {{"fmt", "none"}, {"ecolor", "red"}, {"capsize", "3"}});

    methods.push_back(orderName);
    xg.push_back(xpos);
    i++;
  }

  double oldMean = math_stat::mean(oldTimes);
  tlog_info << oldMean << std::endl;

  plt::plot(
      std::vector<double>{-0.5, xg.back() + 0.5},
      std::vector<double>{oldMean, oldMean},
      {{"linestyle", "-."},
       {"color",
        "#333333"},  // насичений сірий замість фіолетового{"color", "#c5b0d5"},
       {"linewidth", "1.5"},
       {"label", getOriginalTSName()}});

  if (_barData.size() > 9) {
    makeLegentByGroups(getColorGroups(_order), {0.90, 0.9});
  }

  plt::xticks(xg, methods, {{"fontsize", "7"}});
  plt::ylabel(getYAxisName());
  plt::xlabel(getXAxisName());

  if (_barData.size() > 5) {
    rotateXTicks90();
  }

  applyPublicationStyle();

  plt::save(ensureExtension(m_path, ".eps"), 1200);

  plt::pause(0.01);
  plt::close();

  serialize(ensureExtension(m_path, ".bin"));
}

}  // namespace mathplot
}  // namespace cider
