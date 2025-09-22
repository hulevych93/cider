// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "q-learning-cov-ep-plot.h"

#include <assert.h>

#include <tlog.h>

#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

CovQLearningResultsMathplotLogger::CovQLearningResultsMathplotLogger(
    const std::string& logDir,
    const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

void CovQLearningResultsMathplotLogger::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << ieps_;
    serializer << cov_;
    serializer.save(filePath);
  } catch (...) {
    tlog_info << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool CovQLearningResultsMathplotLogger::load() {
  try {
    serialization::Deserializer deserializer(ensureExtension(m_path, ".bin"));
    deserializer >> ieps_;
    deserializer >> cov_;
  } catch (const std::exception& e) {
    tlog_info << e.what() << std::endl;
    return false;
  }
  return true;
}

void CovQLearningResultsMathplotLogger::log(size_t episode, const double cov) {
  ieps_.push_back(episode);
  cov_.push_back(cov);

  if ((_updateCounter++ % 100) == 0) {
    plot();
  }
}

void CovQLearningResultsMathplotLogger::plot() {
  plt::clf();

  applyPublicationStyle();

  plt::plot(std::vector<double>{0, ieps_.back()},
            std::vector<double>{_maxCov, _maxCov},
            {{"linestyle", "-."},
             {"color", "green"},
             {"linewidth", "1.0"},
             {"label", "Pretraining Reward Value"}});

  plt::plot(ieps_, cov_,
            std::map<std::string, std::string>{
                {"color", "red"}, {"linestyle", "-"}, {"linewidth", "0.5"}});

  plt::ylabel("Branch Coverage (%)");
  plt::xlabel("Episode");

  plt::grid(true);

  applyPublicationStyle();
  plt::pause(0.01);
}

CovQLearningResultsMathplotLogger::~CovQLearningResultsMathplotLogger() {
  serialize(ensureExtension(m_path, ".bin"));

  plot();
  plt::save(ensureExtension(m_path, ".eps"), 1200);
}

}  // namespace mathplot
}  // namespace cider
