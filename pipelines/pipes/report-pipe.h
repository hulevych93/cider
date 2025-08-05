// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"

namespace cider {
namespace pipelines {

using ReportConfiguration = std::vector<std::string>;

class StepperReportStage final : public Pipe {
 public:
  explicit StepperReportStage(const ReportConfiguration& config);

 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }

 private:
  ReportConfiguration _config;
};

class BoxPlotReportStage final : public Pipe {
 public:
  explicit BoxPlotReportStage(const ReportConfiguration& config);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }

 private:
  ReportConfiguration _config;
};

class LinesBarPlotReportStage final : public Pipe {
 public:
  explicit LinesBarPlotReportStage(const ReportConfiguration& config);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }

 private:
  ReportConfiguration _config;
};

class HeatmapPlotReportStage final : public Pipe {
 public:
  explicit HeatmapPlotReportStage(const ReportConfiguration& config);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }

 private:
  ReportConfiguration _config;
};

class EfficencyReportStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }
};

class RemoveDataStage final : public Pipe {
 public:
  explicit RemoveDataStage(const ReportConfiguration& config);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }

 private:
  ReportConfiguration _config;
};

class ProcessDataStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }
};

}  // namespace pipelines
}  // namespace cider
