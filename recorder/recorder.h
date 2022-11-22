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
ScriptRecordSessionPtr makeLuaRecordingSession();

}  // namespace recorder
}  // namespace gunit
