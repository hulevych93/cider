#pragma once

#include "details/action.h"

#include "utils/numeric_cast.h"
#include "utils/type_utils.h"

namespace gunit {
namespace recorder {

class UserActionsObserver {
 public:
  virtual ~UserActionsObserver() = default;
  virtual void onUserAction(const UserAction& action) = 0;
  virtual void onEndOfScope() = 0;
};

using UserActionsObserverWeakPtr = std::weak_ptr<UserActionsObserver>;
using UserActionsObserverPtr = std::shared_ptr<UserActionsObserver>;

class ActionScopeGuard final {
 public:
  ActionScopeGuard(UserActionsObserver* observer) : _observer(observer) {}
  ~ActionScopeGuard() {
    if (_observer)
      _observer->onEndOfScope();
  }

  ActionScopeGuard(const ActionScopeGuard&) = delete;
  ActionScopeGuard(ActionScopeGuard&&) = delete;

 private:
  UserActionsObserver* _observer;
};

class UserActionsObservable final {
 public:
  void attachObserver(UserActionsObserverPtr observer) {
    _observer = std::move(observer);
  }

  static UserActionsObservable& getInstance() {
    static UserActionsObservable instance;
    return instance;
  }

  template <typename... Args>
  ActionScopeGuard notifyFreeFunction(const char* function, Args&&... args) {
    if (auto observer = _observer.lock()) {
      try {
        observer->onUserAction(
            freeFunctionCall(function, std::forward<Args>(args)...));
        return ActionScopeGuard{observer.get()};
      } catch (const BadNumCast&) {
        //...
      }
    }
    return ActionScopeGuard{nullptr};
  }

 private:
  UserActionsObservable() = default;

  UserActionsObserverWeakPtr _observer;
};

#define GUNIT_NOTIFY_FREE_FUNCTION(FUNCTION_NAME, ...)                    \
  auto scopeGuard = gunit::recorder::UserActionsObservable::getInstance() \
                        .notifyFreeFunction(FUNCTION_NAME, __VA_ARGS__);

}  // namespace recorder
}  // namespace gunit
