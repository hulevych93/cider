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
    : m_path(ensurePath(logDir, logFileName)) {
  plt::figure_size(640, 640);
}

void QLearningResultsMathplotLogger::logReward(size_t episode,
                                               const double totalReward) const {
  ieps_.push_back(static_cast<double>(episode));
  rwrd_.push_back(totalReward);

  plot();
}

void QLearningResultsMathplotLogger::logLoss(size_t episode,
                                             const double averageLoss) const {
  jeps_.push_back(static_cast<double>(episode));
  loss_.push_back(averageLoss);

  plot();
}

void QLearningResultsMathplotLogger::plot() const {
  plt::clf();

  plt::subplot2grid(2, 1, 0, 0);
  plt::plot(ieps_, rwrd_,
            std::map<std::string, std::string>{{"label", "Total Reward"},
                                               {"color", "red"},
                                               {"linestyle", "-"},
                                               {"linewidth", "0.5"}});
  plt::grid(true);
  plt::legend();

  plt::subplot2grid(2, 1, 1, 0);
  plt::plot(jeps_, loss_,
            std::map<std::string, std::string>{{"label", "Average Loss"},
                                               {"color", "blue"},
                                               {"linestyle", "-"},
                                               {"linewidth", "0.5"}});
  plt::xlabel("Epochs");
  plt::legend();
  plt::grid(true);

  plt::pause(0.01);
}

QLearningResultsMathplotLogger::~QLearningResultsMathplotLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();
}

}  // namespace mathplot
}  // namespace cider
