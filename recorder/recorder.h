#pragma once

#include "actions_observer.h"

namespace gunit {
namespace recorder {

class ScriptRecordSession {
 public:
  virtual ~ScriptRecordSession() = default;
  virtual std::string getScript() = 0;
};

using ScriptRecordSessionPtr = std::shared_ptr<ScriptRecordSession>;
ScriptRecordSessionPtr makeLuaRecordingSession(const std::string& moduleName);

}  // namespace recorder
}  // namespace gunit
