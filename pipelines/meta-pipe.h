// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "coverage/coverage.h"
#include "metaheuristics/metasearch.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

class HarmonySearchStage final : public IPipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input,
               Actions& out) override;
};

class CackooSearchStage final : public IPipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input,
               Actions& out) override;
};

}  // namespace pipelines
}  // namespace cider
