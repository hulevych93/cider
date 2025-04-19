// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include <filesystem>
#include <fstream>
#include <iostream>

#include "coverage/coverage.h"
#include "coverage/measurer.h"

#include "metaheuristics/cuckoo/cuckoo.h"
#include "metaheuristics/harmony/harmony.h"

#include "recorder/details/generator.h"
#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

std::unique_ptr<cider::metasearch::IMetaSearch> makeHarmonySearch(
    cider::coverage::CoverageMeasurment& meassurer);

std::unique_ptr<cider::metasearch::IMetaSearch> makeCackooSearch(
    cider::coverage::CoverageMeasurment& meassurer);

int metaPipeline(const std::string& libName,
                 const cider::coverage::Cmd& cmd,
                 cider::recorder::ScriptRecordSessionPtr session);

}  // namespace pipelines
}  // namespace cider
