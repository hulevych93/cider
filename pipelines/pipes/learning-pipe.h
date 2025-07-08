// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"
#include "recorder/recorder.h"

#include "agent-model/settings.h"

namespace cider {
namespace pipelines {

class PreLearningStage final : public Pipe {
 public:
  PreLearningStage(const agent_model::LearningSettings& settings);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "PL"; }

 private:
  agent_model::LearningSettings m_settings;
};

class LearningStage final : public Pipe {
 public:
  explicit LearningStage(const agent_model::LearningSettings& settings);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "L"; }

 private:
  agent_model::LearningSettings m_settings;
};

}  // namespace pipelines
}  // namespace cider
