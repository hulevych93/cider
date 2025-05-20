// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

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

class QGenerationStage final : public Pipe {
 public:
  QGenerationStage(const qleaning::GenerationSettings& settings,
                   int numberOfRuns = 1);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "G"; }

 private:
  qleaning::QAgent& m_agent;
  qleaning::GenerationSettings m_settings;
  const int _numberOfRuns;
};

class QRandGenerationStage final : public Pipe {
 public:
  QRandGenerationStage(const qleaning::GenerationSettings& settings,
                       int numberOfRuns = 1);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "RNDG"; }

 private:
  qleaning::QAgent& m_agent;
  qleaning::GenerationSettings _settings;
  const int _numberOfRuns;
};

bool generate(const qleaning::GenerationSettings& settings,
              const std::string& libName,
              const cider::Cmd& cmd,
              const recorder::Actions& input,
              recorder::Actions& output);

}  // namespace pipelines
}  // namespace cider
