// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "dataset-pipe.h"

#include "coverage/gcov_measurer.h"

#include <assert.h>

namespace cider {
namespace pipelines {

bool DatasetStage::process(const std::string&,
                           const std::string& libName,
                           const cider::Cmd& cmd) {
  // const auto& input = getInput();

  // cider::gcov_coverage::CoverageMeasurment measurer{cmd, libName.c_str()};

  // const auto report = measurer.getReport(input.actions);
  // assert(report.has_value());

  // pushResult("DATASET", libName.c_str(), input.actions.size(),
  //            input.actions.size(), report->report);
  return true;
}

}  // namespace pipelines
}  // namespace cider
