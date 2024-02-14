// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/actions_observer.h"

namespace cider {
namespace recorder {

struct ScriptGenerationError final : public std::exception {
  explicit ScriptGenerationError(const char* msg);

  const char* what() const noexcept override { return _error.c_str(); }

 private:
  std::string _error;
};

struct SessionSettings final {};

class IScriptRecordSession {
 public:
  virtual ~IScriptRecordSession() = default;
  virtual std::string getScript(size_t) = 0;
  virtual size_t getInstructionsCount() const = 0;
  virtual std::vector<Action> getInstructions() const = 0;
};

using ScriptRecordSessionPtr = std::shared_ptr<IScriptRecordSession>;
ScriptRecordSessionPtr makeLuaRecordingSession(
    const std::string& moduleName,
    const SessionSettings& settings = {});

}  // namespace recorder
}  // namespace cider
