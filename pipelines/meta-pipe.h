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

  std::string getLetter() const override { return "HS"; }
};

class CackooSearchStage final : public IPipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input,
               Actions& out) override;

  std::string getLetter() const override { return "CS"; }
};

class ResetArgumentsStage final : public IPipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input,
               Actions& out) override;

  std::string getLetter() const override { return "RA"; }
};

}  // namespace pipelines
}  // namespace cider
