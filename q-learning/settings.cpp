// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "settings.h"

#include <iostream>
#include <thread>

#include <assert.h>

#include "utils.h"

namespace cider {
namespace qleaning {

std::ostream& operator<<(std::ostream& os, const LearningSettings& settings) {
  os << "QL_";
  os << "lr[";
  os << settings.learningRate;
  os << "]_df[";
  os << settings.discountFactor;
  os << "]_epds[";
  os << settings.episodes;
  os << "],_mxStDp[";
  os << settings.maxStateDepth;
  os << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const GenerationSettings& settings) {
  os << "QL_GEN_";
  std::string strategyName;
  std::string stopType;

  switch (settings.strategy) {
    case GenerationStrategyType::Greedy:
      strategyName = "Greedy";
      break;
    case GenerationStrategyType::EGreedy:
      strategyName = "E-Greedy";
      break;
    case GenerationStrategyType::Boltzmann:
      strategyName = "Boltzmann";
      break;
    default:
      strategyName = "Unknown";
  }

  switch (settings.stopType) {
    case GenerationStopType::LimitActions:
      stopType = "LimitActions";
      break;
    case GenerationStopType::GreaterCoverage:
      stopType = "GreaterCoverage";
      break;
    default:
      strategyName = "Unknown";
  }

  os << "st[" << strategyName;
  os << "]_eps[" << settings.epsilon;
  os << "]_temp[" << settings.temperature;
  os << "]_maxSt[" << settings.maxRollback;
  os << "]_stType[" << stopType;
  os << "]_lim[" << settings.limitActions;
  os << "]";
  return os;
}

}  // namespace qleaning
}  // namespace cider
