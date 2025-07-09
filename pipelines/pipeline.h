// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT
#pragma once

#include "recorder/recorder.h"

#include "coverage/coverage.h"

#include "coverage/gcov_coverage.h"

#include "pipe.h"
#include "results.h"

namespace cider {
namespace pipelines {

class Pipeline final {
  friend class Pipe;

 public:
  Pipeline(const std::string& libName, const cider::Cmd& cmd);

  bool hasResults(const std::string& name) const {
    return _resultsStorage.find(name) != _resultsStorage.end();
  }
  bool hasResults() const { return !_resultsStorage.empty(); }

  void addStage(std::unique_ptr<Pipe> pipe) {
    pipe->setOwner(this);
    _pipes.emplace_back(std::move(pipe));
  }

  bool runOneByOne(
      const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions);

 private:
  bool run(const std::string& metadata);

  bool load(const std::string& filePath);
  bool save(const std::string& filePath) const;

  const std::string pipelineConfig() const;
  static std::string getDatetimeForDirName();

  Results& getResults() { return _resultsStorage; }

 private:
  std::string _libName;
  cider::Cmd _cmd;
  std::vector<std::unique_ptr<Pipe>> _pipes;

  Input _input;
  Results _resultsStorage;
};

Pipeline makePipeline(const std::string& libName, const cider::Cmd& cmd);

}  // namespace pipelines
}  // namespace cider
