// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include <fstream>
#include <iostream>

#include "coverage/coverage.h"
#include "harmony/harmony.h"
#include "harmony/measurer.h"
#include "recorder/details/generator.h"
#include "recorder/recorder.h"

#include <assert.h>

extern int run_tests(const char* temp_);

int main(int argc, char* argv[]) {
  try {
    cider::harmony::Cmd cmd(argc, argv);

    auto session = cider::recorder::makeLuaRecordingSession("pugixml");

    try {
      run_tests(argv[0]);
    } catch (const std::exception& e) {
      std::cout << e.what();
    }

    cider::harmony::Settings settings;
    settings.mutationRate = 0.15;
    settings.harmonyMemoryConsiderationRate = 0.5;
    settings.harmonyMemorySize = 10;
    settings.maxIterationsWithoutUpdates = 500;
    settings.strategy = cider::harmony::MutationStrategy::ChangeBits;

    cider::harmony::CoverageMeasurment measurer{cmd, "log.txt", "pugixml"};
    settings.meassure =
        std::bind(&cider::harmony::CoverageMeasurment::operator(),
                  std::ref(measurer), std::placeholders::_1);

    cider::harmony::Search hs{settings};
    std::cout << "InstructionsCount: " << session->getInstructionsCount()
              << std::endl;
    hs.initialize(session->getInstructions());
    hs.run();

    const auto& bestActions = hs.getBest().actions;

    auto generator = cider::recorder::makeLuaGenerator("pugixml");
    const auto script =
        cider::recorder::generateScript(generator, bestActions, 99999U);

    std::ofstream output_file("optimized_pugi.lua");
    output_file << script;

    cider::harmony::StepperCoverageMeasurment stepper{cmd, "pugixml"};
    stepper.measure(bestActions);

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}
