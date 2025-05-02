// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "report-pipe.h"

#include "coverage/cfg_measurer.h"
#include "coverage/coverage.h"
#include "coverage/gcov_measurer.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

bool ReportStage::process(const std::string& metadata,
                          const std::string& libName,
                          const cider::Cmd& cmd,
                          const Actions& input,
                          Actions& output) {
  {
    cider::gcov_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};
    auto fileLog = std::make_unique<cider::gcov_coverage::FileLogger>(
        cmd.resultsDir + '/' + metadata, "report.txt");
    measurer.setLogger(std::move(fileLog));

    const auto objFunc =
        [&](const std::vector<cider::recorder::Action>& actions) -> double {
      const auto rootReport = measurer(actions);
      if (rootReport.has_value()) {
        return rootReport->report.lineCov.percent;
      }
      return 0.0f;
    };

    objFunc(input);
  }

  output = input;
  return true;
}

}  // namespace pipelines
}  // namespace cider
