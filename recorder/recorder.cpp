#include "recorder.h"

#include "details/action.h"
#include "details/generator.h"
#include "details/lua/lua_func.h"
#include "details/lua/lua_params.h"

#include <deque>

namespace gunit {
namespace recorder {

class ScriptRecordSessionImpl final : public ScriptRecordSession,
                                      public ActionsObserver {
  struct ActionEntry final {
    Action action;
    action_id id;
    size_t nestingLevel;
  };

 public:
  explicit ScriptRecordSessionImpl(ScriptGenerator&& generator)
      : _generator(std::move(generator)) {}

  std::string getScript() override {
    try {
      for (const auto& entry : _log) {
        if (entry.nestingLevel < 1u) {
          std::visit(_generator, entry.action);
        }
      }
      reset();
      return _generator.getScript();
    } catch (...) {
        reset();
        _generator.discard();
        throw;
    }
  }

 private:
  action_id onActionBegins(const Action& action) override {
    const auto id = _action_id;
    _log.emplace_back(ActionEntry{action, id, _nestingLevel});
    ++_nestingLevel;
    ++_action_id;
    return id;
  }

  void onActionEnds(const action_id) override { --_nestingLevel; }

  void reset() {
      _log.clear();
      _nestingLevel = 0u;
      _action_id = 1u;
  }

 private:
  std::deque<ActionEntry> _log;
  size_t _nestingLevel = 0u;
  action_id _action_id = 1u;

  ScriptGenerator _generator;
};

ScriptRecordSessionPtr makeLuaRecordingSession(const std::string& moduleName) {
  LanguageContext context;
  context.funcProducer = produceFunctionCall;
  context.paramProducer = produceLuaCode;
  context.binaryOpProducer = produceBinaryOpCall;
  ScriptGenerator generator{moduleName, context};

  auto session =
      std::make_shared<ScriptRecordSessionImpl>(std::move(generator));
  ActionsObservable::getInstance().attachObserver(session);
  return session;
}

}  // namespace recorder
}  // namespace gunit
