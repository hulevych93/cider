// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "greedy-r-pipe.h"

namespace cider {
namespace pipelines {

namespace {

template <typename SettingsType>
bool runRGreedy(SettingsType settings,
                const ObjectiveFunction& objFunc,
                const ObjectiveFunction& objFuncСfg,
                const FineObjectiveFunction& fineObjFunc,
                const recorder::Actions& input,
                recorder::Actions& output,
                double baseline,
                std::vector<bool>& openers) {
  try {
    if constexpr (std::is_same_v<SettingsType, greedy_r::GreedyRSettings>) {
      settings.objFunc = objFunc;
      settings.baseline = baseline;
      settings.objFuncCfg = objFuncСfg;

      output = greedy_r::run_greedy_r(Seed::instance().get(), settings, input);
    } else if constexpr (std::is_same_v<SettingsType,
                                        greedy_r::GreedyRTracksSettings>) {
      settings.objFunc = objFunc;
      settings.fineObjFunc = fineObjFunc;
      settings.baseline = baseline;
      settings.openers = std::addressof(openers);

      output = greedy_r::run_greedy_r_tracks(Seed::instance().get(), settings,
                                             input);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return false;
  }

  return true;
}

}  // namespace

GreedyRStage::GreedyRStage(const greedy_r::GreedySettings& settings,
                           int numberOfRuns)
    : SimulationPipe(numberOfRuns), m_settings(settings) {}

bool GreedyRStage::simulate(const std::string& /*outPath*/,
                            const double baseline,
                            const recorder::Actions& input,
                            recorder::Actions& output,
                            const ObjectiveFunction& objFunc,
                            const ObjectiveFunction& objFuncСfg,
                            const FineObjectiveFunction& fineObjFunc) {
  return std::visit(
      [&](const auto& settings) -> bool {
        return runRGreedy(settings, objFunc, objFuncСfg, fineObjFunc,
                          deepCopy(input), output, baseline, _openers);
      },
      m_settings);
}

std::string GreedyRStage::getPrefix() const {
  std::stringstream os;
  std::visit([&os](const auto& s) { os << "_" << s; }, m_settings);
  return os.str();
}

std::string GreedyRStage::getConfigName() const {
  return std::visit([](const auto& settings) { return settings.configName; },
                    m_settings);
}

}  // namespace pipelines
}  // namespace cider
