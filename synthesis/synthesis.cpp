// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "synthesis.h"

#include <tlog.h>
#include <thread>

#include <assert.h>
#include <random>

namespace cider {
namespace synthesis {
std::ostream& operator<<(std::ostream& os,
                         const SynthesisSettingsBasic& settings) {
  os << "QL_GEN_";
  std::string stopType;

  switch (settings.stopType) {
    case synthesis::StopCondition::LimitActions:
      stopType = "LimitActions";
      break;
    case synthesis::StopCondition::GreaterCoverage:
      stopType = "GreaterCoverage";
      break;
  }

  os << "maxSt[" << settings.maxRollback;
  os << "]_stType[" << stopType;
  os << "]_lim[" << settings.limitActions;
  os << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os,
                         const AgentSynthesisSettings& settings) {
  os << "GEN_";
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
    case synthesis::StopCondition::LimitActions:
      stopType = "LimitActions";
      break;
    case synthesis::StopCondition::GreaterCoverage:
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

namespace details {

bool synthesize(const SynthesisSettingsBasic& settings,
                const ActionChoosing& actionChoosing,
                TestScenario& testCase) {
  size_t rollbackCount = 0;

  auto stopPredicate = [&]() -> bool {
    if (settings.stopType == StopCondition::GreaterCoverage) {
      return testCase.isOver();
    }

    if (settings.stopType == StopCondition::LimitActions) {
      return testCase.getSize() >= settings.limitActions;
    }

    throw std::runtime_error{"Wrong stop"};
  };

  while (!stopPredicate()) {
    const auto selectedOpt = actionChoosing(testCase);

    if (!selectedOpt.has_value()) {
      tlog_info << "No selected action" << std::endl;
      break;
    }

    const auto selected = selectedOpt.value();

    testCase.add(selected);
    if (testCase.isValid()) {
      rollbackCount = 0U;
    } else if (testCase.isOver()) {
      break;
    } else {
      testCase.rollback();
      ++rollbackCount;
      if (rollbackCount > settings.maxRollback) {
        tlog_info << "Max rollback reached [" << settings.maxRollback << "]"
                  << std::endl;
        break;
      }
    }
  }

  return true;
}

}  // namespace details

}  // namespace synthesis
}  // namespace cider
