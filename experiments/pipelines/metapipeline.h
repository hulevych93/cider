// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "coverage/coverage.h"
#include "coverage/measurer.h"

#include "metaheuristics/cuckoo/cuckoo.h"
#include "metaheuristics/harmony/harmony.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

std::unique_ptr<cider::metasearch::IMetaSearch> makeHarmonySearch(
    cider::coverage::CoverageMeasurment& meassurer,
    cider::metasearch::MutationStrategy stategy);

std::unique_ptr<cider::metasearch::IMetaSearch> makeCackooSearch(
    cider::coverage::CoverageMeasurment& meassurer,
    cider::metasearch::MutationStrategy stategy);

int metaPipeline(const std::string& libName,
                 const cider::coverage::Cmd& cmd,
                 cider::recorder::ScriptRecordSessionPtr session,
                 cider::metasearch::MutationStrategy stategy =
                     cider::metasearch::MutationStrategy::ShuffleBytes);

}  // namespace pipelines
}  // namespace cider
