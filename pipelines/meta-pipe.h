// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "coverage/coverage.h"
#include "metaheuristics/metasearch.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

class HarmonySearchStage final : public Pipe {
 public:
  HarmonySearchStage(const metasearch::harmony::Settings& settings);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "HS"; }

 private:
  metasearch::harmony::Settings m_settings;
};

class CackooSearchStage final : public Pipe {
 public:
  CackooSearchStage(const metasearch::cuckoo::Settings& settings);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "CS"; }

 private:
  metasearch::cuckoo::Settings m_settings;
};

}  // namespace pipelines
}  // namespace cider
