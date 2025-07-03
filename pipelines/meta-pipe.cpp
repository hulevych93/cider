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

namespace cider {
namespace pipelines {

namespace {

template <typename CoverageMetricsType, typename SettingsType>
bool makeMetaPipeline(SettingsType& settings,
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
    metaSearch->setLogger(std::make_unique<cider::mathplot::MathplotLogger>(
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

HarmonySearchStage::HarmonySearchStage(
    const metasearch::harmony::Settings& settings)
    : m_settings(settings) {}

bool HarmonySearchStage::process(const std::string& metadata,
                                 const std::string& libName,
                                 const cider::Cmd& cmd) {
  std::function<recorder::Actions()> callback;

  std::string dataName;

  const auto& input = getInput();
  if (input.actionsCallback) {
    dataName = "HSQL";
    callback = input.actionsCallback;
  } else {
    dataName = "HS";
    callback = [input]() { return deepCopy(input.actions); };
  }

  recorder::Actions output;
  auto res = makeMetaPipeline<cfg_coverage::CoverageMeasurment>(
      m_settings, metadata, libName, cmd, callback, output);
  pushResult(dataName.c_str(), libName.c_str(), output);

  cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd, libName.c_str()};

  const auto report = gcov_measurer.getReport(output);
  if (report.has_value()) {
    pushResult(dataName.c_str(), libName.c_str(), input.actions.size(),
               output.size(), report->report);
  }

  return res;
}

CackooSearchStage::CackooSearchStage(
    const metasearch::cuckoo::Settings& settings)
    : m_settings(settings) {}

bool CackooSearchStage::process(const std::string& metadata,
                                const std::string& libName,
                                const cider::Cmd& cmd) {
  std::function<recorder::Actions()> callback;

  std::string dataName;

  const auto& input = getInput();
  if (input.actionsCallback) {
    dataName = "CSQL";
    callback = input.actionsCallback;
  } else {
    dataName = "CS";
    callback = [input]() { return deepCopy(input.actions); };
  }

  recorder::Actions output;
  auto res = makeMetaPipeline<cfg_coverage::CoverageMeasurment>(
      m_settings, metadata, libName, cmd, callback, output);
  pushResult(dataName.c_str(), libName.c_str(), output);

  cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd, libName.c_str()};

  const auto report = gcov_measurer.getReport(output);
  if (report.has_value()) {
    pushResult(dataName.c_str(), libName.c_str(), input.actions.size(),
               output.size(), report->report);
  }

  return res;
}

namespace {
recorder::Actions resetArgs(const recorder::Actions& input) {
  auto result = deepCopy(input);
  auto nuller = recorder::makeNullableMutator();
  for (auto& action : result) {
    recorder::ActionMutator mutator(*nuller);
    std::visit(mutator, action);
  }
  return result;
}
}  // namespace

}  // namespace pipelines
}  // namespace cider
