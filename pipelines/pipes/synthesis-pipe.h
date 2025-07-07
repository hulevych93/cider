// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"
#include "recorder/recorder.h"

#include "synthesis/synthesis.h"

namespace cider {
namespace pipelines {

class SynthesisStage final : public Pipe {
 public:
  SynthesisStage(const synthesis::SynthesisSettings& settings,
                 int numberOfRuns = 1);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "G"; }

 private:
  std::string getConfigName() const;

  synthesis::SynthesisSettings m_settings;
  const int _numberOfRuns;
};

}  // namespace pipelines
}  // namespace cider
