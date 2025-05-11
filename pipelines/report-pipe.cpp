// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "report-pipe.h"

#include "coverage/cfg_measurer.h"
#include "coverage/coverage.h"
#include "coverage/gcov_measurer.h"

#include "recorder/details/generator.h"
#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

bool ReportStage::process(const std::string& metadata,
                          const std::string& libName,
                          const cider::Cmd& cmd,
                          const Actions& input,
                          Actions& output) {
  std::filesystem::path outPath(cmd.resultsDir);
  outPath /= metadata;

  auto generator = cider::recorder::makeLuaGenerator(libName);

  {
    const auto script =
        cider::recorder::generateScript(generator, input, 99999U);

    std::ofstream output_file(outPath / (libName + ".lua"));
    output_file << script;
  }

  {
    const auto script =
        cider::recorder::generateScript(generator, output, 99999U);

    std::ofstream output_file(outPath / ("optimized_" + libName + ".lua"));
    output_file << script;
  }

  {
    cider::gcov_coverage::StepperCoverageMeasurment stepper{cmd,
                                                            libName.c_str()};

    stepper.setLogger(outPath.string(), "initial_log.txt");

    stepper.measure(input);
  }

  {
    cider::gcov_coverage::StepperCoverageMeasurment stepper{cmd,
                                                            libName.c_str()};

    stepper.setLogger(outPath.string(), "optimized_log.txt");

    stepper.measure(output);
  }

  return true;
}

}  // namespace pipelines
}  // namespace cider
