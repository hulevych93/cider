#pragma once

#include "details/action.h"

#include "utils/numeric_cast.h"
#include "utils/type_utils.h"

namespace gunit {
namespace recorder {

// TODO: fix id
using action_id = size_t;
constexpr action_id null_action_id = 0u;

class ActionsObserver {
 public:
  virtual ~ActionsObserver() = default;
  virtual action_id onActionBegins(const Action& action) = 0;
  virtual void onActionEnds(action_id Id) = 0;
};

using ActionsObserverWeakPtr = std::weak_ptr<ActionsObserver>;
using ActionsObserverPtr = std::shared_ptr<ActionsObserver>;

class ActionScopeGuard final {
 public:
  ActionScopeGuard(action_id id, ActionsObserver* observer)
      : _Id(id), _observer(observer) {}
  ~ActionScopeGuard() {
    if (_observer)
      _observer->onActionEnds(_Id);
  }

  ActionScopeGuard(const ActionScopeGuard&) = delete;
  ActionScopeGuard(ActionScopeGuard&&) = delete;

 private:
  action_id _Id;
  ActionsObserver* _observer;
};

class ActionsObservable final {
 public:
  void attachObserver(ActionsObserverPtr observer) {
    _observer = std::move(observer);
  }

  static ActionsObservable& getInstance() {
    static ActionsObservable instance;
    return instance;
  }

  template <typename... Args>
  ActionScopeGuard notifyFreeFunction(const char* function, Args&&... args) {
    if (auto observer = _observer.lock()) {
      try {
        const auto Id = observer->onActionBegins(
            makeFreeFunctionCall(function, std::forward<Args>(args)...));
        return ActionScopeGuard{Id, observer.get()};
      } catch (const BadNumCast&) {
        //...
      }
    }
    return ActionScopeGuard{null_action_id, nullptr};
  }

 private:
  ActionsObservable() = default;

  ActionsObserverWeakPtr _observer;
};

#define GUNIT_NOTIFY_FREE_FUNCTION(FUNCTION_NAME, ...)                      \
  auto scopeGuard =                                                         \
      gunit::recorder::ActionsObservable::getInstance().notifyFreeFunction( \
          FUNCTION_NAME, __VA_ARGS__);

}  // namespace recorder
}  // namespace gunit
