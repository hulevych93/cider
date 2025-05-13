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
  QPreLearningStage();

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input) override;

  std::string getLetter() const override { return "QPL"; }

 private:
  qleaning::QAgent& m_agent;
};

class QLearningStage final : public Pipe {
 public:
  QLearningStage();

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input) override;

  std::string getLetter() const override { return "QL"; }

 private:
  qleaning::QAgent& m_agent;
};

class QGenerationStage final : public Pipe {
 public:
  QGenerationStage();

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input) override;

  std::string getLetter() const override { return "G"; }

 private:
  qleaning::QAgent& m_agent;
};

class QRandGenerationStage final : public Pipe {
 public:
  QRandGenerationStage();

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input) override;

  std::string getLetter() const override { return "RNDG"; }

 private:
  qleaning::QAgent& m_agent;
};

class GenerationReportStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input) override;

  std::string getLetter() const override { return "RP"; }
};

}  // namespace pipelines
}  // namespace cider
