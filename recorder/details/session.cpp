#include "session.h"

namespace cider {
namespace recorder {

ScriptRecordSessionImpl::ScriptRecordSessionImpl(
    ScriptGenerator&& generator,
    const SessionSettings& settings)
    : _generator(std::move(generator)), _settings(settings) {}

std::string ScriptRecordSessionImpl::getScript() {
  try {
    size_t count = 0U;
    for (const auto& entry : _log) {
      if (entry.nestingLevel < 1u) {
        std::visit(_generator, entry.action);
        ++count;
        if (count >= _settings.insructionsCount) {
          break;
        }
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

action_id ScriptRecordSessionImpl::onActionBegins(const Action& action) {
  const auto id = _action_id;
  _log.emplace_back(ActionEntry{action, id, _nestingLevel});
  ++_nestingLevel;
  ++_action_id;
  return id;
}  // LCOV_EXCL_LINE

void ScriptRecordSessionImpl::onActionEnds(const action_id) {
  --_nestingLevel;
}  // LCOV_EXCL_LINE

void ScriptRecordSessionImpl::reset() {
  _log.clear();
  _nestingLevel = 0u;
  _action_id = 1u;
}

}  // namespace recorder
}  // namespace cider
