// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "meta-pipe.h"

#include <tlog.h>
#include <filesystem>
#include <fstream>

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "metaheuristics/cuckoo/cuckoo.h"
#include "metaheuristics/harmony/harmony.h"
#include "metaheuristics/harmony/harmony_synthesis.h"

#include "mathplot-log/monitoring/metasearch-basic-block-cov-plot.h"

#include <assert.h>

namespace cider {
namespace pipelines {

namespace {

template <typename SettingsType>
bool makeMetaPipeline(SettingsType settings,
                      const std::string& outPath,
                      const ObjectiveFunction& objFunc,
                      const FineObjectiveFunction& fineObjFunc,
                      const std::function<recorder::Actions()>& callback,
                      recorder::Actions& output) {
  std::unique_ptr<cider::metasearch::IMetaSearch> metaSearch;

  settings.objFunc = objFunc;
  settings.fineObjFunc = fineObjFunc;

  if constexpr (std::is_same_v<SettingsType, metasearch::harmony::Settings>) {
    metaSearch = std::make_unique<metasearch::harmony::Search>(settings);
  } else if constexpr (std::is_same_v<SettingsType,
                                      metasearch::cuckoo::Settings>) {
    metaSearch = std::make_unique<metasearch::cuckoo::Search>(settings);
  } else if constexpr (std::is_same_v<
                           SettingsType,
                           metasearch::harmony_synthesis::Settings>) {
    metaSearch =
        std::make_unique<metasearch::harmony_synthesis::Search>(settings);
  }

  metaSearch->setLogger(std::make_unique<cider::mathplot::BasicBlockCovLogger>(
      outPath, "meta_search.png"));

  metaSearch->initialize(callback);
  metaSearch->run();

  output = metaSearch->getBest().actions;

  return true;
}

}  // namespace

MetaSearchStage::MetaSearchStage(const metasearch::MetaSettings& settings,
                                 int numberOfRuns)
    : SimulationPipe(numberOfRuns), m_settings(settings) {}

bool MetaSearchStage::simulate(const std::string& outPath,
                               const double /*baseline*/,
                               const recorder::Actions& input,
                               recorder::Actions& output,
                               const ObjectiveFunction& objFunc,
                               const ObjectiveFunction& /*objFuncСfg*/,
                               const FineObjectiveFunction& fineObjFunc) {
  return std::visit(
      [&](const auto& s) {
        return makeMetaPipeline(
            s, outPath, objFunc, fineObjFunc, [&]() { return deepCopy(input); },
            output);
      },
      m_settings);
}

std::string MetaSearchStage::getPrefix() const {
  std::stringstream os;
  std::visit([&os](const auto& s) { os << "_" << s; }, m_settings);
  return os.str();
}

std::string MetaSearchStage::getConfigName() const {
  return std::visit([&](const auto& settings) { return settings.configName; },
                    m_settings);
}

}  // namespace pipelines
}  // namespace cider
