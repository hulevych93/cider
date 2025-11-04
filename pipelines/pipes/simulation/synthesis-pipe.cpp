// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "synthesis-pipe.h"

#include "synthesis/agent-synthesis.h"
#include "synthesis/rand-synthesis.h"

#include "agent-q-learning/q-learning-agent.h"
#include "mathplot-log/monitoring/suffix-exist-rate-plot.h"
#include "mathplot-log/monitoring/suffix-freq-plot.h"

#include <assert.h>
#include <tlog.h>

namespace cider {
namespace pipelines {

SynthesisStage::SynthesisStage(const synthesis::SynthesisSettings& settings,
                               int numberOfRuns)
    : SimulationPipe(numberOfRuns), m_settings(settings) {}

SynthesisStage::~SynthesisStage() = default;

bool SynthesisStage::simulate(const std::string& outPath,
                              const double /*baseline*/,
                              const recorder::Actions& input,
                              recorder::Actions& output,
                              const ObjectiveFunction& objFunc,
                              const ObjectiveFunction& /*objFuncСfg*/,
                              const FineObjectiveFunction& fineObjFunc) {
  if (!_freqPlot) {
    std::string graphTitle = "freq_plot_";
    graphTitle += getConfigName();

    auto& existPlot =
        mathplot::SuffixExistRatePlot::get(outPath, "freq_exist_plot");

    existPlot.next(std::visit(
        [&](const auto& settings) -> double {
          using T = std::decay_t<decltype(settings)>;
          if constexpr (std::is_same_v<T, synthesis::QSynthesisSettings>) {
            return settings.lambda;
          } else {
            return 0.0;
          }
        },
        m_settings));

    _freqPlot = std::make_unique<mathplot::SuffixFreqPlot>(outPath, graphTitle);
    agent_model::qlearning::QLearningAgent::setLogger(
        [&](const auto suffix, bool found) {
          _freqPlot->log(suffix);
          existPlot.log(suffix, found);
        });
  }

  return std::visit(
      [&](const auto& s) {
        return synthesis::synthesize(Seed::instance().get(), s, objFunc,
                                     fineObjFunc, input, output);
      },
      m_settings);
}

void SynthesisStage::onCleanup() {
  _freqPlot->plot();

  auto& existPlot = mathplot::SuffixExistRatePlot::get();
  existPlot.finalize();
}

std::string SynthesisStage::getPrefix() const {
  std::stringstream os;
  std::visit([&os](const auto& s) { os << "_" << s; }, m_settings);
  return os.str();
}

std::string SynthesisStage::getConfigName() const {
  return std::visit([&](const auto& settings) { return settings.configName; },
                    m_settings);
}

}  // namespace pipelines
}  // namespace cider
