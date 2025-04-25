// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "coverage/coverage.h"
#include "coverage/measurer.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

int qlearningPipeline(
    const std::string& libName,
    const cider::coverage::Cmd& cmd,
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions);

}  // namespace pipelines
}  // namespace cider
