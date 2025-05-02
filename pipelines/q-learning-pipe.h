// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "coverage/coverage.h"
#include "recorder/recorder.h"

#include "q-learning/q-learning.h"

namespace cider {
namespace pipelines {

class QLearningStage final : public IPipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd,
               const Actions& input,
               Actions& out) override;

 private:
  qleaning::QValuesAgent m_agent;
};

}  // namespace pipelines
}  // namespace cider
