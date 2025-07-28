// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning-reward-loss-plot.h"

#include <assert.h>

#include <iostream>

#ifdef ENABLE_MATHPLOT
#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;
#endif

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

QLearningResultsMathplotLogger::QLearningResultsMathplotLogger(
    const std::string& logDir,
    const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)), _plot(PlotType::Reward) {}

void QLearningResultsMathplotLogger::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << ieps_;
    serializer << rwrd_;
    serializer << jeps_;
    serializer << loss_;
    serializer.save(filePath);
  } catch (...) {
    std::cout << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool QLearningResultsMathplotLogger::load() {
  try {
    serialization::Deserializer deserializer(ensureBinExtension(m_path));
    deserializer >> ieps_;
    deserializer >> rwrd_;
    deserializer >> jeps_;
    deserializer >> loss_;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

void QLearningResultsMathplotLogger::logReward(size_t episode,
                                               const double totalReward) {
  ieps_.push_back(static_cast<double>(episode));
  rwrd_.push_back(totalReward);

  plot();
}

void QLearningResultsMathplotLogger::logLoss(size_t episode,
                                             const double averageLoss) {
  jeps_.push_back(static_cast<double>(episode));
  loss_.push_back(averageLoss);

  plot();
}

void QLearningResultsMathplotLogger::plot() {
  plt::clf();

  applyPublicationStyle();

  if (_plot == PlotType::Reward) {
    plt::plot(ieps_, rwrd_,
              std::map<std::string, std::string>{
                  {"color", "red"}, {"linestyle", "-"}, {"linewidth", "0.5"}});

  } else {
    plt::plot(jeps_, loss_,
              std::map<std::string, std::string>{
                  {"color", "blue"}, {"linestyle", "-"}, {"linewidth", "0.5"}});
  }

  plt::ylabel(_plot == PlotType::Reward ? "Total Reward (Unit)"
                                        : "Average Loss (Unit)");
  plt::xlabel("Episode");

  plt::xlim(0.0, 1000.0);
  plt::grid(true);

  applyPublicationStyle();
  plt::pause(0.01);
}

QLearningResultsMathplotLogger::~QLearningResultsMathplotLogger() {
  save();
  plt::close();
}

void QLearningResultsMathplotLogger::save() {
  serialize(ensureBinExtension(m_path));

  if (_plot == PlotType::Reward) {
    m_path += "_r";
  } else {
    m_path += "_l";
  }

  plt::save(ensurePngExtension(m_path), 1200);
}

}  // namespace mathplot
}  // namespace cider
