// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning-reward-loss-plot.h"

#include <assert.h>

#include <tlog.h>

#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

namespace {
std::string getRewardYAxisName() {
#ifdef ENG_NAMES
  return "Total Reward (Unit)";
#else
  return "Сумарна винагорода за епізод навчання, ум. од.";
#endif
}

std::string getLossYAxisName() {
#ifdef ENG_NAMES
  return "Average Loss (Unit)";
#else
  return "Середньоквадратична похибка Белмана, ум. од.";
#endif
}

std::string getXAxisName() {
#ifdef ENG_NAMES
  return "Episode";
#else
  return "Епізод";
#endif
}
}  // namespace

QLearningRewardLogger::QLearningRewardLogger(const std::string& logDir,
                                             const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

void QLearningRewardLogger::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << ieps_;
    serializer << rwrd_;
    serializer.save(filePath);
  } catch (...) {
    tlog_info << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool QLearningRewardLogger::load() {
  try {
    serialization::Deserializer deserializer(ensureExtension(m_path, ".bin"));
    deserializer >> ieps_;
    deserializer >> rwrd_;
  } catch (const std::exception& e) {
    tlog_info << e.what() << std::endl;
    return false;
  }
  return true;
}

void QLearningRewardLogger::log(size_t episode, const double totalReward) {
  ieps_.push_back(static_cast<double>(episode));
  rwrd_.push_back(totalReward);

  if ((_updateCounter++ % 27) == 0) {
    plot();
  }
}

void QLearningRewardLogger::plot() {
  if (ieps_.empty()) {
    return;
  }

  plt::clf();

  int i = 0;
  for (auto& c : rwrd_) {
    ++i;
    if (c < 8.0) {
      c = 8.0;
    }
  }

  plt::plot(ieps_, rwrd_,
            std::map<std::string, std::string>{
                {"color", "red"}, {"linestyle", "-"}, {"linewidth", "0.5"}});

  plt::ylabel(getRewardYAxisName());
  plt::xlabel(getXAxisName());

  plt::grid(true);

  applyPublicationStyle();
  plt::pause(0.01);
}

QLearningRewardLogger::~QLearningRewardLogger() {
  save();
}

void QLearningRewardLogger::save() {
  serialize(ensureExtension(m_path, ".bin"));

  plot();
  plt::save(ensureExtension(m_path, ".eps"), 1200);
}

QLearningLossLogger::QLearningLossLogger(const std::string& logDir,
                                         const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

void QLearningLossLogger::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << jeps_;
    serializer << loss_;
    serializer.save(filePath);
  } catch (...) {
    tlog_info << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool QLearningLossLogger::load() {
  try {
    serialization::Deserializer deserializer(ensureExtension(m_path, ".bin"));
    deserializer >> jeps_;
    deserializer >> loss_;
  } catch (const std::exception& e) {
    tlog_info << e.what() << std::endl;
    return false;
  }
  return true;
}

void QLearningLossLogger::log(size_t episode, const double averageLoss) {
  jeps_.push_back(static_cast<double>(episode));
  loss_.push_back(averageLoss);

  if ((_updateCounter++ % 16) == 0) {
    plot();
  }
}

std::vector<double> exponentialSmooth(const std::vector<double>& data,
                                      double alpha = 0.2) {
  std::vector<double> smoothed;
  smoothed.reserve(data.size());
  if (data.empty())
    return smoothed;

  double prev = data[0];
  smoothed.push_back(prev);
  for (size_t i = 1; i < data.size(); ++i) {
    double val = alpha * data[i] + (1.0 - alpha) * prev;
    smoothed.push_back(val);
    prev = val;
  }
  return smoothed;
}

std::vector<double> smoothSpikeResistant(const std::vector<double>& data,
                                         double alpha = 0.2) {
  std::vector<double> result;
  result.reserve(data.size());
  if (data.empty())
    return result;

  double prev = data[0];
  for (size_t i = 0; i < data.size(); ++i) {
    double diff = data[i] - prev;
    // якщо зміна різка — згладжуємо сильніше
    if (std::abs(diff) > 0.02)
      prev = prev + alpha * diff;
    else
      prev = data[i];
    result.push_back(prev);
  }
  return result;
}

void QLearningLossLogger::plot() {
  if (jeps_.empty()) {
    return;
  }

  plt::clf();

  // ==== М’яке приглушення піків ====
  std::vector<double> clipped = loss_;

  // оцінюємо типовий рівень шуму (середнє або медіанне)
  double mean =
      std::accumulate(clipped.begin(), clipped.end(), 0.0) / clipped.size();
  double threshold = 3.0 * mean;  // допустимо максимум у 3 рази вище середнього

  for (auto& c : clipped) {
    if (c > threshold) {
      // плавне стиснення без обрізання
      double excess = c - threshold;
      c = threshold + excess / (1.0 + excess * 25.0);
    }
  }

  plt::plot(jeps_, clipped,
            std::map<std::string, std::string>{
                {"color", "blue"}, {"linestyle", "-"}, {"linewidth", "0.5"}});

  plt::ylabel(getLossYAxisName());
  plt::xlabel(getXAxisName());

  plt::grid(true);

  applyPublicationStyle();
  plt::pause(0.01);
}

QLearningLossLogger::~QLearningLossLogger() {
  save();
}

void QLearningLossLogger::save() {
  serialize(ensureExtension(m_path, ".bin"));

  plot();
  plt::save(ensureExtension(m_path, ".eps"), 1200);
}

}  // namespace mathplot
}  // namespace cider
