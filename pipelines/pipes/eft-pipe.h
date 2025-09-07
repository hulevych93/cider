// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"

namespace cider {
namespace pipelines {

class EfficencyReportStage final : public Pipe {
 public:
  explicit EfficencyReportStage(const ReportConfiguration& config);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }

  bool needTS() const override { return true; }

 private:
  ReportConfiguration _config;
};

}  // namespace pipelines
}  // namespace cider
