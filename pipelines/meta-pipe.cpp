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

#ifdef ENABLE_MATHPLOT
#include "mathplot-log/mathplot-log.h"
#endif

namespace cider {
namespace pipelines {

namespace {

std::unique_ptr<cider::metasearch::IMetaSearch> makeHarmonySearch(
    const cider::metasearch::ObjectiveFunction& objFunc,
    std::string& filePrefix) {
  cider::metasearch::harmony::Settings settings;
  settings.mutationRate = 0.05;
  settings.harmonyMemoryConsiderationRate = 0.4;
  settings.harmonyMemorySize = 1;
  settings.maxIterationsWithoutUpdates = 200;
  settings.strategy = cider::metasearch::MutationStrategy::ShuffleBytes;
  settings.objFunc = objFunc;

  std::stringstream os;
  os << settings;
  filePrefix = os.str();

  return std::make_unique<cider::metasearch::harmony::Search>(settings);
}

std::unique_ptr<cider::metasearch::IMetaSearch> makeCackooSearch(
    const cider::metasearch::ObjectiveFunction& objFunc,
    std::string& filePrefix) {
  cider::metasearch::cuckoo::Settings settings;
  settings.populationSize = 1;
  settings.Pa = 0.1;
  settings.maxIterationsWithoutUpdates = 200;
  settings.strategy = cider::metasearch::MutationStrategy::LevyFlight;
  settings.objFunc = objFunc;

  std::stringstream os;
  os << settings;
  filePrefix = os.str();

  return std::make_unique<cider::metasearch::cuckoo::Search>(settings);
}

template <typename CoverageMetricsType>
bool makeMetaPipeline(PipelineType type,
                      const std::string& metadata,
                      const std::string& libName,
                      const cider::Cmd& cmd,
                      const Actions& input,
                      Actions& output) {
  try {
    CoverageMetricsType measurer{cmd, libName.c_str()};

    std::unique_ptr<cider::metasearch::IMetaSearch> metaSearch;

    std::string prefix;
    switch (type) {
      case cider::PipelineType::HarmonySearch:
        metaSearch = makeHarmonySearch(std::ref(measurer), prefix);
        break;
      case cider::PipelineType::CackooSearch:
        metaSearch = makeCackooSearch(std::ref(measurer), prefix);
        break;
      default:
        throw std::logic_error{"wrong meta algorithm"};
    }

    std::filesystem::path outPath(cmd.resultsDir);
    outPath /= metadata;
    outPath /= prefix;

    measurer.setLogger(outPath.string(), "meta_log.txt");

#ifdef ENABLE_MATHPLOT
    metaSearch->setLogger(std::make_unique<cider::metasearch::MathplotLogger>(
        outPath, "meta_search.png"));
#endif

    metaSearch->initialize(input);
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

bool HarmonySearchStage::process(const std::string& metadata,
                                 const std::string& libName,
                                 const cider::Cmd& cmd,
                                 const Actions& input,
                                 Actions& out) {
  return makeMetaPipeline<cfg_coverage::CoverageMeasurment>(
      PipelineType::HarmonySearch, metadata, libName, cmd, input, out);
}

bool CackooSearchStage::process(const std::string& metadata,
                                const std::string& libName,
                                const cider::Cmd& cmd,
                                const Actions& input,
                                Actions& out) {
  return makeMetaPipeline<cfg_coverage::CoverageMeasurment>(
      PipelineType::CackooSearch, metadata, libName, cmd, input, out);
}

bool ResetArgumentsStage::process(const std::string& /*metadata*/,
                                  const std::string& /*libName*/,
                                  const cider::Cmd& /*cmd*/,
                                  const Actions& input,
                                  Actions& out) {
  auto myInput = deepCopy(input);
  auto nuller = recorder::makeNullableMutator();
  for (auto& action : myInput) {
    recorder::ActionMutator mutator(*nuller);
    std::visit(mutator, action);
  }
  out = myInput;
  return true;
}

}  // namespace pipelines
}  // namespace cider
