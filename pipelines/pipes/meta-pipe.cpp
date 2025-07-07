// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "meta-pipe.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "metaheuristics/cuckoo/cuckoo.h"
#include "metaheuristics/harmony/harmony.h"

#include "recorder/details/generator.h"
#include "recorder/recorder.h"

#ifdef ENABLE_MATHPLOT
#include "mathplot-log/monitoring/metasearch-basic-block-cov-plot.h"
#endif

#include <assert.h>

namespace cider {
namespace pipelines {

namespace {

template <typename CoverageMetricsType, typename SettingsType>
bool makeMetaPipeline(SettingsType settings,
                      const std::string& metadata,
                      const std::string& libName,
                      const cider::Cmd& cmd,
                      const std::function<recorder::Actions()>& callback,
                      recorder::Actions& output) {
  try {
    CoverageMetricsType measurer{cmd, libName.c_str()};

    std::unique_ptr<cider::metasearch::IMetaSearch> metaSearch;

    std::stringstream os;
    os << settings;
    auto prefix = os.str();

    settings.objFunc = std::ref(measurer);

    if constexpr (std::is_same_v<SettingsType, metasearch::harmony::Settings>) {
      metaSearch = std::make_unique<metasearch::harmony::Search>(settings);
    } else if constexpr (std::is_same_v<SettingsType,
                                        metasearch::cuckoo::Settings>) {
      metaSearch = std::make_unique<metasearch::cuckoo::Search>(settings);
    }

    std::filesystem::path outPath(cmd.resultsDir);
    outPath /= metadata;
    outPath /= prefix;

    measurer.setLogger(outPath.string(), "meta_log.txt");

#ifdef ENABLE_MATHPLOT
    metaSearch->setLogger(
        std::make_unique<cider::mathplot::BasicBlockCovLogger>(
            outPath, "meta_search.png"));
#endif

    metaSearch->initialize(callback);
    metaSearch->run();

    const auto& bestActions = metaSearch->getBest().actions;

    output = bestActions;
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return false;
  }

  return true;
}

}  // namespace

MetaSearchStage::MetaSearchStage(const metasearch::MetaSettings& settings,
                                 int numberOfRuns)
    : m_settings(settings), _numberOfRuns(numberOfRuns) {}

bool MetaSearchStage::process(const std::string& metadata,
                              const std::string& libName,
                              const cider::Cmd& cmd) {
  bool success = false;
  const auto& input = getInput();

  for (int idx = 0; idx < _numberOfRuns; ++idx) {
    auto start = std::chrono::steady_clock::now();
    recorder::Actions output;

    std::visit(
        [&](const auto& settings) {
          success = makeMetaPipeline<cfg_coverage::CoverageMeasurment>(
              settings, metadata, libName, cmd,
              [input]() { return deepCopy(input.actions); }, output);
        },
        m_settings);

    auto end = std::chrono::steady_clock::now();
    unsigned long elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    Result result;
    result.testCaseName = input.testOrLibName;
    result.timeElapsedMs = elapsed_ms;
    result.oldActions = deepCopy(input.actions);
    result.newActions = deepCopy(output);

    pushResult(libName.c_str(), cmd, getConfigName(), result);
  }

  return success;
}

std::string MetaSearchStage::getConfigName() const {
  return std::visit([&](const auto& settings) { return settings.configName; },
                    m_settings);
}

}  // namespace pipelines
}  // namespace cider
