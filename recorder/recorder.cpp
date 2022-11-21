#include "recorder.h"

#include "details/action.h"
#include "details/generator.h"

#include <deque>

namespace gunit {
namespace recorder {

class ScriptRecordSessionImpl final : public ScriptRecordSession,
                                      public UserActionsObserver {
 public:
  struct ActionEntry final {
    UserAction action;
    size_t nestingLevel;
  };

  std::string getScript() const override {
    ScriptGenerator generator;

    for (const auto& entry : _log) {
      try {
        if (entry.nestingLevel < 1u) {
          std::visit(generator, entry.action);
        }
      } catch (ScriptGenerationError&) {
        //
      }
    }

    return generator.getScript();
  }

  void onUserAction(const UserAction& action) override {
    _log.emplace_back(ActionEntry{action, _nestingLevel});
    ++_nestingLevel;
  }

  void onEndOfScope() override { --_nestingLevel; }

 private:
  std::deque<ActionEntry> _log;
  size_t _nestingLevel = 0;
};

ScriptRecordSessionPtr makeRecordingSession() {
  auto session = std::make_shared<ScriptRecordSessionImpl>();
  UserActionsObservable::getInstance().attachObserver(session);
  return session;
}

}  // namespace recorder
}  // namespace gunit
