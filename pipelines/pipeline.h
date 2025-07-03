// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT
#pragma once

#include "coverage/coverage.h"

#include "coverage/gcov_coverage.h"

#include "metaheuristics/metasearch.h"

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

#include "recorder/recorder.h"

#include "results.h"

#include <unordered_map>

namespace cider {
namespace pipelines {

class Pipeline;

class Pipe {
 public:
  virtual ~Pipe() = default;

  virtual bool process(const std::string& metadata,
                       const std::string& libName,
                       const cider::Cmd& cmd) = 0;

  virtual std::string getLetter() const = 0;

 protected:
  friend class Pipeline;

  void setOwner(Pipeline* owner) { _owner = owner; }

  void pushResult(const char* libName,
                  const std::vector<recorder::Actions>& sessions);

  void pushResult(const char* methodName,
                  const char* libOrTestName,
                  const recorder::Actions& actions);

  void pushResult(const char* methodName,
                  const char* libOrTestName,
                  int oldLines,
                  int newLines,
                  const gcov_coverage::CoverageReport& report);

  const Input& getInput() const;
  const Results& getResults() const;
  const BriefResults& getBriefResults() const;
  const SessionsResult& getSessionsResults() const;

 private:
  Pipeline* _owner = nullptr;
};

class Pipeline final {
  friend class Pipe;

 public:
  Pipeline(const std::string& libName, const cider::Cmd& cmd);

  bool hasBrieft() const { return !_briefResultsStorage.empty(); }

  void setType(PipelineType type) { _type = type; }

  void addStage(std::unique_ptr<Pipe> pipe) {
    pipe->setOwner(this);
    _pipes.emplace_back(std::move(pipe));
  }

  bool runOneByOne(
      const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions);

  bool runAll(
      const std::vector<cider::recorder::ScriptRecordSessionPtr>& sessions);

 private:
  bool run(const std::string& metadata);

  bool load(const std::string& filePath);
  bool save(const std::string& filePath) const;

  bool loadBrief(const std::string& filePath);
  bool saveBrief(const std::string& filePath) const;

  bool loadSessions(const std::string& filePath);
  bool saveSessions(const std::string& filePath) const;

  const std::string pipelineConfig() const;
  static std::string getDatetimeForDirName();

  Results& getResults() { return _resultsStorage[_type]; }
  BriefResults& getBriefResults() { return _briefResultsStorage[_type]; }
  SessionsResults& getSessionsResults() {
    return _sessionsResultsStorage[_type];
  }

 private:
  PipelineType _type;
  std::string _libName;
  cider::Cmd _cmd;
  std::vector<std::unique_ptr<Pipe>> _pipes;

  Input _input;

  ResultsStorage _resultsStorage;
  BriefResultStorage _briefResultsStorage;
  SessionsResultStorage _sessionsResultsStorage;
};

Pipeline makePipeline(const std::string& libName, const cider::Cmd& cmd);

}  // namespace pipelines
}  // namespace cider
