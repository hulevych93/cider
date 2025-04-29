// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "metapipeline.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "metaheuristics/cuckoo/cuckoo.h"
#include "metaheuristics/harmony/harmony.h"

namespace cider {
namespace pipelines {

std::unique_ptr<cider::metasearch::IMetaSearch> makeHarmonySearch(
    const cider::metasearch::ObjectiveFunction& objFunc,
    std::string& filePrefix) {
  cider::metasearch::harmony::Settings settings;
  settings.mutationRate = 0.05;
  settings.harmonyMemoryConsiderationRate = 0.2;
  settings.harmonyMemorySize = 5;
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
  settings.populationSize = 5;
  settings.Pa = 0.1;
  settings.maxIterationsWithoutUpdates = 200;
  settings.strategy = cider::metasearch::MutationStrategy::LevyFlight;
  settings.objFunc = objFunc;

  std::stringstream os;
  os << settings;
  filePrefix = os.str();

  return std::make_unique<cider::metasearch::cuckoo::Search>(settings);
}

int metaPipeline(const std::string& libName,
                 const cider::Cmd& cmd,
                 const std::string& prefix,
                 std::unique_ptr<cider::metasearch::IMetaSearch> metaSearch,
                 cider::recorder::ScriptRecordSessionPtr session) {
  try {
    std::cout << "InstructionsCount: " << session->getInstructionsCount()
              << std::endl;

    metaSearch->initialize(session->getInstructions());
    metaSearch->run();
    const auto& bestActions = metaSearch->getBest().actions;

    auto generator = cider::recorder::makeLuaGenerator(libName);
    const auto script =
        cider::recorder::generateScript(generator, bestActions, 99999U);

    std::filesystem::path optimizedScriptPath = cmd.resultsDir;
    std::ofstream output_file(optimizedScriptPath /
                              (prefix + "_optimized_" + libName + ".lua"));
    output_file << script;

    std::cout << "Stepper works[" << libName << "]" << std::endl;

    {
      cider::gcov_coverage::CoverageMeasurment stepper{cmd, libName.c_str()};

      auto fileLog = std::make_unique<cider::gcov_coverage::FileLogger>(
          cmd.resultsDir, prefix + "_" + "gcov_nitial_stepper_log.txt");
      stepper.setLogger(std::move(fileLog));

      stepper(session->getInstructions());
    }

    {
      cider::gcov_coverage::CoverageMeasurment stepper{cmd, libName.c_str()};

      auto fileLog = std::make_unique<cider::gcov_coverage::FileLogger>(
          cmd.resultsDir, prefix + "_" + "gcov_optimized_stepper_log.txt");
      stepper.setLogger(std::move(fileLog));

      stepper(bestActions);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}

int metaGcovrPipeline(const std::string& libName,
                      const cider::Cmd& cmd,
                      cider::recorder::ScriptRecordSessionPtr session) {
  cider::gcov_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};

  const auto objFunc =
      [&](const std::vector<cider::recorder::Action>& actions) -> double {
    const auto rootReport = measurer(actions);
    if (rootReport.has_value()) {
      return rootReport->report.lineCov.percent;
    }
    return 0.0f;
  };

  std::unique_ptr<cider::metasearch::IMetaSearch> metaSearch;

  std::string prefix;
  if (cmd.pipelineType == cider::PipelineType::HarmonySearch) {
    metaSearch = makeHarmonySearch(objFunc, prefix);
  } else {
    metaSearch = makeCackooSearch(objFunc, prefix);
  }

  auto fileLog = std::make_unique<cider::gcov_coverage::FileLogger>(
      cmd.resultsDir, prefix + "_gcov_meta_log.txt");
  measurer.setLogger(std::move(fileLog));

  return metaPipeline(libName, cmd, prefix, std::move(metaSearch), session);
}

int metaCfgPipeline(const std::string& libName,
                    const cider::Cmd& cmd,
                    cider::recorder::ScriptRecordSessionPtr session) {
  cider::cfg_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};

  const auto objFunc =
      [&](const std::vector<cider::recorder::Action>& actions) -> double {
    const auto rootReport = measurer(actions);
    if (rootReport.has_value()) {
      return rootReport->getPercentage();
    }
    return 0.0f;
  };

  std::unique_ptr<cider::metasearch::IMetaSearch> metaSearch;

  std::string prefix;
  if (cmd.pipelineType == cider::PipelineType::HarmonySearch) {
    metaSearch = makeHarmonySearch(objFunc, prefix);
  } else {
    metaSearch = makeCackooSearch(objFunc, prefix);
  }

  auto fileLog = std::make_unique<cider::cfg_coverage::FileLogger>(
      cmd.resultsDir, prefix + "_cfg_meta_log.txt");
  measurer.setLogger(std::move(fileLog));

  return metaPipeline(libName, cmd, prefix, std::move(metaSearch), session);
}

}  // namespace pipelines
}  // namespace cider
