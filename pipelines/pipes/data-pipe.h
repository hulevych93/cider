// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include "coverage/coverage.h"

namespace cider {
namespace pipelines {

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

class ShowResultsStage final : public Pipe {
 public:
  explicit ShowResultsStage(const ReportConfiguration& config);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }

 private:
  ReportConfiguration _config;
};

class CleanUpDataStage final : public Pipe {
 public:
  explicit CleanUpDataStage(const ReportConfiguration& config);

  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }

 private:
  ReportConfiguration _config;
};

class ModifyDataStage final : public Pipe {
 public:
  bool process(const std::string& metadata,
               const std::string& libName,
               const cider::Cmd& cmd) override;

  std::string getLetter() const override { return "REPORT"; }
};

}  // namespace pipelines
}  // namespace cider
