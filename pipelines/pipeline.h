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
};

class Pipeline final {
 public:
  Pipeline(const std::string& libName, const cider::Cmd& cmd);

  void addStage(std::unique_ptr<IPipe> pipe) {
    _pipes.emplace_back(std::move(pipe));
  }

  bool run(const Actions& input, Actions& output) {
    Actions in = deepCopy(input);
    Actions out;
    const auto dateTime = getDatetimeForDirName();
    for (auto& pipe : _pipes) {
      if (!pipe->process(dateTime, _libName, _cmd, in, out)) {
        return false;
      }
      in = deepCopy(out);
    }
    _report->process(dateTime, _libName, _cmd, input, out);
    output = out;
    return true;
  }

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
