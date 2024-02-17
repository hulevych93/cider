// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "session.h"
#include <iostream>

namespace cider {
namespace recorder {

ScriptRecordSessionImpl::ScriptRecordSessionImpl(
    ScriptGenerator&& generator,
    const SessionSettings& settings)
    : _generator(std::move(generator)), _settings(settings) {}

std::string ScriptRecordSessionImpl::getScript(const size_t instuctions) {
  try {
    size_t count = 0U;
    for (const auto& entry : _log) {
      try {
        if (entry.nestingLevel < 1u) {
          std::visit(_generator, entry.action);
          ++count;
          if (count >= instuctions) {
            break;
          }
        }

      } catch (const std::exception& ex) {
        std::cout << "Call generation ignored, what: " << ex.what()
                  << std::endl;
      }
    }
    return _generator.getScript();
  } catch (...) {
    reset();
    _generator.discard();
    throw;
  }
}

size_t ScriptRecordSessionImpl::getInstructionsCount() const {
  size_t count = 0U;
  for (const auto& entry : _log) {
    if (entry.nestingLevel < 1u) {
      ++count;
    }
  }
  return count;
}

std::vector<Action> ScriptRecordSessionImpl::getInstructions() const {
  std::vector<Action> actions;
  actions.reserve(getInstructionsCount());
  for (const auto& entry : _log) {
    if (entry.nestingLevel < 1u) {
      actions.push_back(entry.action);
    }
  }
  return actions;
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
