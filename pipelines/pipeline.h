// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT
#pragma once

#include "coverage/coverage.h"
#include "metaheuristics/metasearch.h"

#include "recorder/recorder.h"

namespace cider {
namespace pipelines {

using Actions = std::vector<recorder::Action>;
using Results = std::vector<Actions>;

class Pipeline;

class Pipe {
 public:
  virtual ~Pipe() = default;

  virtual bool process(const std::string& metadata,
                       const std::string& libName,
                       const cider::Cmd& cmd,
                       const Actions& input) = 0;

  virtual std::string getLetter() const = 0;

 protected:
  friend class Pipeline;

  void setOwner(Pipeline* owner) { _owner = owner; }

  void pushResult(const Actions& result);
  const Results& getResults() const;

 private:
  Pipeline* _owner = nullptr;
};

class Pipeline final {
  friend class Pipe;

 public:
  Pipeline(const std::string& libName, const cider::Cmd& cmd);

  void addStage(std::unique_ptr<Pipe> pipe) {
    pipe->setOwner(this);
    _pipes.emplace_back(std::move(pipe));
  }

  bool run(
      const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions);

 private:
  bool run(const std::string& metadata, const Actions& input);

  const std::string pipelineConfig() const;
  static std::string getDatetimeForDirName();

 private:
  std::string _libName;
  cider::Cmd _cmd;
  std::vector<std::unique_ptr<Pipe>> _pipes;
  std::vector<Actions> _results;
};

Pipeline makePipeline(const std::string& libName, const cider::Cmd& cmd);

}  // namespace pipelines
}  // namespace cider
