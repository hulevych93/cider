// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "coverage/coverage.h"
#include "metaheuristics/metasearch.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

int metaGcovrPipeline(const std::string& libName,
                      const cider::Cmd& cmd,
                      cider::recorder::ScriptRecordSessionPtr session);

int metaCfgPipeline(const std::string& libName,
                    const cider::Cmd& cmd,
                    cider::recorder::ScriptRecordSessionPtr session);

}  // namespace pipelines
}  // namespace cider
