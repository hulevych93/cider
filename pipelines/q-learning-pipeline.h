// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "coverage/coverage.h"
#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

int qlearningGcovrPipeline(
    const std::string& libName,
    const cider::Cmd& cmd,
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions);

int qlearningCfgPipeline(
    const std::string& libName,
    const cider::Cmd& cmd,
    const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions);

}  // namespace pipelines
}  // namespace cider
