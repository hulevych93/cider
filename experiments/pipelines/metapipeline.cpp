// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "metapipeline.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "recorder/details/generator.h"

namespace cider {
namespace pipelines {

std::unique_ptr<cider::metasearch::IMetaSearch> makeHarmonySearch(
    cider::coverage::CoverageMeasurment& meassurer,
    cider::metasearch::MutationStrategy stategy) {
  cider::metasearch::harmony::Settings settings;
  settings.mutationRate = 0.01;
  settings.harmonyMemoryConsiderationRate = 0.5;
  settings.harmonyMemorySize = 1;
  settings.maxIterationsWithoutUpdates = 500;
  settings.strategy = stategy;

  settings.meassure =
      std::bind(&cider::coverage::CoverageMeasurment::operator(),
                std::ref(meassurer), std::placeholders::_1);

  return std::make_unique<cider::metasearch::harmony::Search>(settings);
}

std::unique_ptr<cider::metasearch::IMetaSearch> makeCackooSearch(
    cider::coverage::CoverageMeasurment& meassurer,
    cider::metasearch::MutationStrategy stategy) {
  cider::metasearch::cuckoo::Settings settings;
  settings.populationSize = 5;
  settings.Pa = 0.1;
  settings.maxIterationsWithoutUpdates = 500;
  settings.strategy = stategy;

  settings.meassure =
      std::bind(&cider::coverage::CoverageMeasurment::operator(),
                std::ref(meassurer), std::placeholders::_1);

  return std::make_unique<cider::metasearch::cuckoo::Search>(settings);
}

int metaPipeline(const std::string& libName,
                 const cider::coverage::Cmd& cmd,
                 cider::recorder::ScriptRecordSessionPtr session,
                 cider::metasearch::MutationStrategy stategy) {
  try {
    std::cout << "InstructionsCount: " << session->getInstructionsCount()
              << std::endl;

    cider::coverage::CoverageMeasurment measurer{cmd, "log.txt",
                                                 libName.c_str()};

    std::unique_ptr<cider::metasearch::IMetaSearch> metaSearch;

    if (false) {
      metaSearch = makeHarmonySearch(measurer, stategy);
    } else {
      metaSearch = makeCackooSearch(measurer, stategy);
    }

    metaSearch->initialize(session->getInstructions());
    metaSearch->run();
    const auto& bestActions = metaSearch->getBest().actions;

    auto generator = cider::recorder::makeLuaGenerator(libName);
    const auto script =
        cider::recorder::generateScript(generator, bestActions, 99999U);

    std::ofstream output_file(std::string{"optimized_"} + libName + ".lua");
    output_file << script;

    std::cout << "Stepper works[" << libName << "]" << std::endl;
    cider::coverage::StepperCoverageMeasurment stepper{cmd, libName.c_str()};
    stepper.measure(bestActions);

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}

}  // namespace pipelines
}  // namespace cider
