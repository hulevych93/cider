// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning-reward-loss-plot.h"

#include <assert.h>

#include <iostream>

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
    std::cout << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool QLearningRewardLogger::load() {
  try {
    serialization::Deserializer deserializer(ensureBinExtension(m_path));
    deserializer >> ieps_;
    deserializer >> rwrd_;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

void QLearningRewardLogger::log(size_t episode, const double totalReward) {
  ieps_.push_back(static_cast<double>(episode));
  rwrd_.push_back(totalReward);
}

void QLearningRewardLogger::plot() {
  plt::clf();

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
  serialize(ensureBinExtension(m_path));

  plot();
  plt::save(ensurePngExtension(m_path), 1200);
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
    std::cout << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool QLearningLossLogger::load() {
  try {
    serialization::Deserializer deserializer(ensureBinExtension(m_path));
    deserializer >> jeps_;
    deserializer >> loss_;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

void QLearningLossLogger::log(size_t episode, const double averageLoss) {
  jeps_.push_back(static_cast<double>(episode));
  loss_.push_back(averageLoss);
}

void QLearningLossLogger::plot() {
  plt::clf();

  plt::plot(jeps_, loss_,
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
  serialize(ensureBinExtension(m_path));

  plot();
  plt::save(ensurePngExtension(m_path), 1200);
}

}  // namespace mathplot
}  // namespace cider
