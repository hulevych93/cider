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

struct SessionSettings final {
  size_t insructionsCount = 5000U;
};

class IScriptRecordSession {
 public:
  virtual ~IScriptRecordSession() = default;
  virtual std::string getScript() = 0;
};

using ScriptRecordSessionPtr = std::shared_ptr<IScriptRecordSession>;
ScriptRecordSessionPtr makeLuaRecordingSession(
    const std::string& moduleName,
    const SessionSettings& settings = {});

}  // namespace recorder
}  // namespace cider
