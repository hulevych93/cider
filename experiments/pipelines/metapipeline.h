// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "coverage/coverage.h"
#include "metaheuristics/args_mutator.h"
#include "metaheuristics/metasearch.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

int metaGcovrPipeline(const std::string& libName,
                      const cider::Cmd& cmd,
                      cider::recorder::ScriptRecordSessionPtr session,
                      cider::metasearch::MutationStrategy stategy =
                          cider::metasearch::MutationStrategy::ShuffleBytes);

int metaCfgPipeline(const std::string& libName,
                    const cider::Cmd& cmd,
                    cider::recorder::ScriptRecordSessionPtr session,
                    cider::metasearch::MutationStrategy stategy =
                        cider::metasearch::MutationStrategy::ShuffleBytes);

}  // namespace pipelines
}  // namespace cider
