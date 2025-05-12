// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "coverage/coverage.h"
#include "recorder/recorder.h"

#include "q-learning/q-learning.h"

namespace cider {
namespace pipelines {

class QPreLearningStage final : public IPipe {
 public:
  QPreLearningStage();

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input,
               Actions& out) override;

  std::string getLetter() const override { return "QPL"; }

 private:
  qleaning::QAgent& m_agent;
};

class QLearningStage final : public IPipe {
 public:
  QLearningStage();

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input,
               Actions& out) override;

  std::string getLetter() const override { return "QL"; }

 private:
  qleaning::QAgent& m_agent;
};

class QGenerationStage final : public IPipe {
 public:
  QGenerationStage();

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input,
               Actions& out) override;

  std::string getLetter() const override { return "G"; }

 private:
  qleaning::QAgent& m_agent;
};

}  // namespace pipelines
}  // namespace cider
