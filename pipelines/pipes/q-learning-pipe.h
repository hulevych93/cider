// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"
#include "recorder/recorder.h"

#include "q-learning/q-learning.h"

namespace cider {
namespace pipelines {

class QPreLearningStage final : public Pipe {
 public:
  QPreLearningStage(const qleaning::LearningSettings& settings);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "QPL"; }

 private:
  qleaning::LearningSettings m_settings;
  qleaning::QAgent& m_agent;
};

class QLearningStage final : public Pipe {
 public:
  explicit QLearningStage(const qleaning::LearningSettings& settings);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "QL"; }

 private:
  qleaning::LearningSettings m_settings;
  qleaning::QAgent& m_agent;
};

}  // namespace pipelines
}  // namespace cider
