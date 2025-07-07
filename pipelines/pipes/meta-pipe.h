// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"
#include "metaheuristics/metasearch.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

class MetaSearchStage final : public Pipe {
 public:
  MetaSearchStage(const metasearch::MetaSettings& settings,
                  int numberOfRuns = 1);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "HS"; }

 private:
  std::string getConfigName() const;

  metasearch::MetaSettings m_settings;
  const int _numberOfRuns;
};

}  // namespace pipelines
}  // namespace cider
