// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT
#pragma once

#include "recorder/recorder.h"

#include "coverage/coverage.h"

#include "coverage/gcov_coverage.h"

#include "pipe.h"
#include "results.h"

using SessionsGetter =
    std::function<std::vector<cider::recorder::ScriptRecordSessionPtr>()>;

namespace cider {
namespace pipelines {

class Pipeline final {
  friend class Pipe;

 public:
  Pipeline(const std::string& libName, const cider::Cmd& cmd);

  bool hasResults(const std::string& name) const {
    return _results.find(name) != _results.end();
  }
  bool hasResults() const { return !_results.empty(); }

  bool newResuls() const { return _resultsChanged; }

  void addStage(std::unique_ptr<Pipe> pipe) {
    pipe->setOwner(this);
    _pipes.emplace_back(std::move(pipe));
  }

  bool run(SessionsGetter getTS, SessionsGetter getTCs);

  bool save();

  bool save();

 private:
  bool run(const std::string& metadata);

  bool load(const std::string& filePath);
  bool save(const std::string& filePath) const;

  const std::string pipelineConfig() const;
  static std::string getDatetimeForDirName();

  const Results& getResults() const { return _results; }
  Results& getMutableResults() { return _results; }

  void pushResult(const std::string& name, const Result& result) {
    _results[name].emplace_back(std::move(result));
    _resultsChanged = true;
  }

  void clearResults(const std::string& name) {
    _results[name] = {};
    _resultsChanged = true;
  }

 private:
  void clearTrash();

  std::string _libName;
  cider::Cmd _cmd;
  std::vector<std::unique_ptr<Pipe>> _pipes;

  Input _input;
  Results _results;
  bool _isLoaded = false;
  bool _resultsChanged = false;
};

Pipeline makePipeline(const std::string& libName, const cider::Cmd& cmd);

}  // namespace pipelines
}  // namespace cider
