#pragma once

#include "recorder/actions_observer.h"
#include "recorder/recorder.h"

#include "recorder/details/generator.h"

#include <deque>

namespace cider {
namespace recorder {

class ScriptGenerator;

class ScriptRecordSessionImpl final : public ScriptRecordSession,
                                      public ActionsObserver {
  struct ActionEntry final {
    Action action;
    action_id id;
    size_t nestingLevel;
  };

 public:
  explicit ScriptRecordSessionImpl(ScriptGenerator&& generator);

  std::string getScript() override;

 private:
  action_id onActionBegins(const Action& action) override;
  void onActionEnds(const action_id) override;

  void reset();

 private:
  std::deque<ActionEntry> _log;
  size_t _nestingLevel = 0u;
  action_id _action_id = 1u;

  ScriptGenerator _generator;
};

}  // namespace recorder
}  // namespace cider
