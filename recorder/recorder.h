// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/actions_observer.h"

#include <iostream>

namespace cider {
namespace recorder {

struct ScriptGenerationError final : public std::exception {
  explicit ScriptGenerationError(const char* msg);

  const char* what() const noexcept override { return _error.c_str(); }

 private:
  std::string _error;
};

struct SessionSettings final {
  bool printLines = false;
  bool enableGC = false;
  bool ignoreErrors = false;
  bool enableMarks = false;
  int numActions = 0;
  std::string testName;
};

class IScriptRecordSession {
 public:
  virtual ~IScriptRecordSession() = default;
  virtual std::string getScript(size_t) = 0;
  virtual size_t getInstructionsCount() const = 0;
  virtual std::vector<Action> getInstructions() const = 0;

  virtual std::string getName() const = 0;
};

using ScriptRecordSessionPtr = std::shared_ptr<IScriptRecordSession>;
ScriptRecordSessionPtr makeLuaRecordingSession(
    const std::string& moduleName,
    const SessionSettings& settings = {});

template <typename F>
auto recordScript(const char* moduleName,
                  const char* testName,
                  std::vector<cider::recorder::ScriptRecordSessionPtr>& out,
                  F&& f) {
  SessionSettings settings;
  settings.testName = testName;
  auto session = cider::recorder::makeLuaRecordingSession(moduleName, settings);
  try {
    f();
  } catch (const std::exception& e) {
    std::cout << e.what();
  }
  out.emplace_back(std::move(session));
}

template <typename F>
auto recordScriptWithResult(
    const char* moduleName,
    const char* testName,
    std::vector<cider::recorder::ScriptRecordSessionPtr>& out,
    F&& f) {
  SessionSettings settings;
  settings.testName = testName;
  auto session = cider::recorder::makeLuaRecordingSession(moduleName, settings);
  try {
    auto res = f();
    out.emplace_back(std::move(session));
    return res;

  } catch (const std::exception& e) {
    std::cout << e.what();
    throw;
  }
}

#define RECORD_TEST_SCRIPT(MODULE_NAME, TEST_NAME, OUTPUT)         \
  cider::recorder::recordScript(MODULE_NAME, #TEST_NAME, sessions, \
                                []() { TEST_NAME(); });

}  // namespace recorder
}  // namespace cider
