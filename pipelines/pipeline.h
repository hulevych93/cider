// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT
#pragma once

#include "coverage/coverage.h"
#include "metaheuristics/metasearch.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

using Actions = std::vector<recorder::Action>;

class IPipe {
 public:
  virtual ~IPipe() = default;

  virtual bool process(const std::string& metadata,
                       const std::string& libName,
                       const cider::Cmd& cmd,
                       const Actions& input,
                       Actions& out) = 0;

  virtual std::string getLetter() const = 0;
};

class Pipeline final {
 public:
  Pipeline(const std::string& libName, const cider::Cmd& cmd);

  void addStage(std::unique_ptr<IPipe> pipe) {
    _pipes.emplace_back(std::move(pipe));
  }

  void enableReport();

  bool run(
      const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions);

 private:
  bool run(const std::string& metadata, const Actions& input);

  const std::string pipelineConfig() const;
  static std::string getDatetimeForDirName();

 private:
  std::string _libName;
  cider::Cmd _cmd;
  std::vector<std::unique_ptr<IPipe>> _pipes;

  std::unique_ptr<IPipe> _report;
};

Pipeline makePipeline(const std::string& libName, const cider::Cmd& cmd);

}  // namespace pipelines
}  // namespace cider
