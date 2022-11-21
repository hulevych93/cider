#pragma once

#include "actions_observer.h"

namespace gunit {
namespace recorder {

class ScriptRecordSession {
 public:
  virtual ~ScriptRecordSession() = default;
  virtual std::string getScript() const = 0;
};

using ScriptRecordSessionPtr = std::shared_ptr<ScriptRecordSession>;
ScriptRecordSessionPtr makeRecordingSession();

}  // namespace recorder
}  // namespace gunit
