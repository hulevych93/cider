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
    cider::metasearch::MutationStrategy stategy) {
  cider::metasearch::harmony::Settings settings;
  settings.mutationRate = 0.05;
  settings.harmonyMemoryConsiderationRate = 0.2;
  settings.harmonyMemorySize = 5;
  settings.maxIterationsWithoutUpdates = 200;
  settings.strategy = stategy;
  settings.objFunc = objFunc;

  return std::make_unique<cider::metasearch::harmony::Search>(settings);
}

std::unique_ptr<cider::metasearch::IMetaSearch> makeCackooSearch(
    const cider::metasearch::ObjectiveFunction& objFunc,
    cider::metasearch::MutationStrategy stategy) {
  cider::metasearch::cuckoo::Settings settings;
  settings.populationSize = 5;
  settings.Pa = 0.1;
  settings.maxIterationsWithoutUpdates = 500;
  settings.strategy = stategy;
  settings.objFunc = objFunc;

  return std::make_unique<cider::metasearch::cuckoo::Search>(settings);
}

int metaPipeline(const std::string& libName,
                 const cider::Cmd& cmd,
                 const cider::metasearch::ObjectiveFunction& objFunc,
                 cider::recorder::ScriptRecordSessionPtr session,
                 cider::metasearch::MutationStrategy stategy) {
  try {
    std::cout << "InstructionsCount: " << session->getInstructionsCount()
              << std::endl;

    std::unique_ptr<cider::metasearch::IMetaSearch> metaSearch;

    if (true) {
      metaSearch = makeHarmonySearch(objFunc, stategy);
    } else {
      metaSearch = makeCackooSearch(objFunc, stategy);
    }

    std::cout << "Stepper works[" << libName << "]" << std::endl;
    cider::gcov_coverage::CoverageMeasurment stepper{cmd, "meta_stepper.txt",
                                                     libName.c_str()};
    stepper(session->getInstructions());

    metaSearch->initialize(session->getInstructions());
    metaSearch->run();
    const auto& bestActions = metaSearch->getBest().actions;

    auto generator = cider::recorder::makeLuaGenerator(libName);
    const auto script =
        cider::recorder::generateScript(generator, bestActions, 99999U);

    std::ofstream output_file(std::string{"optimized_"} + libName + ".lua");
    output_file << script;

    stepper(bestActions);

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}

int metaGcovrPipeline(const std::string& libName,
                      const cider::Cmd& cmd,
                      cider::recorder::ScriptRecordSessionPtr session,
                      cider::metasearch::MutationStrategy stategy) {
  std::string log = "gcov_meta_log.txt";
  cider::gcov_coverage::CoverageMeasurment measurer{cmd, log.c_str(),
                                                    libName.c_str()};

  const auto objFunc =
      [&](const std::vector<cider::recorder::Action>& actions) -> double {
    const auto rootReport = measurer(actions);
    if (rootReport.has_value()) {
      return rootReport->report.lineCov.percent;
    }
    return 0.0f;
  };

  return metaPipeline(libName, cmd, objFunc, session, stategy);
}

int metaCfgPipeline(const std::string& libName,
                    const cider::Cmd& cmd,
                    cider::recorder::ScriptRecordSessionPtr session,
                    cider::metasearch::MutationStrategy stategy) {
  std::string log = "cfg_meta_log.txt";
  cider::cfg_coverage::CoverageMeasurment measurer{cmd, log.c_str(),
                                                   libName.c_str()};

  const auto objFunc =
      [&](const std::vector<cider::recorder::Action>& actions) -> double {
    const auto rootReport = measurer(actions);
    if (rootReport.has_value()) {
      return rootReport->getPercentage();
    }
    return 0.0f;
  };

  return metaPipeline(libName, cmd, objFunc, session, stategy);
}

}  // namespace pipelines
}  // namespace cider
