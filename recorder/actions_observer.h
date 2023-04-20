#pragma once

#include "recorder/details/action.h"

namespace cider {
namespace recorder {

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
  ActionScopeGuard notify(Args&&... args) {
    if (auto observer = _observer.lock()) {
      const auto Id =
          observer->onActionBegins(makeAction(std::forward<Args>(args)...));
      return ActionScopeGuard{Id, observer.get()};
    }
    return ActionScopeGuard{null_action_id, nullptr};
  }

 private:
  ActionsObservable() = default;

  ActionsObserverWeakPtr _observer;
};

#define CIDER_NOTIFY_FREE_FUNCTION_NO_RETURN(...)                             \
  auto scopeGuard = cider::recorder::ActionsObservable::getInstance().notify( \
      __FUNCTION__, std::nullopt, __VA_ARGS__);

#define CIDER_NOTIFY_FREE_FUNCTION_NO_ARGS(RETURN_VALUE)                      \
  auto scopeGuard = cider::recorder::ActionsObservable::getInstance().notify( \
      __FUNCTION__, RETURN_VALUE);

#define CIDER_NOTIFY_FREE_FUNCTION(RETURN_VALUE, ...)                         \
  auto scopeGuard = cider::recorder::ActionsObservable::getInstance().notify( \
      __FUNCTION__, RETURN_VALUE, __VA_ARGS__);

#define CIDER_NOTIFY_METHOD_NO_RETURN(...)                                    \
  auto scopeGuard = cider::recorder::ActionsObservable::getInstance().notify( \
      _impl.get(), __FUNCTION__, std::nullopt, __VA_ARGS__);

#define CIDER_NOTIFY_METHOD_NO_ARGS(RETURN_VALUE)                             \
  auto scopeGuard = cider::recorder::ActionsObservable::getInstance().notify( \
      _impl.get(), __FUNCTION__, RETURN_VALUE);

#define CIDER_NOTIFY_METHOD(...)                                              \
  auto scopeGuard = cider::recorder::ActionsObservable::getInstance().notify( \
      _impl.get(), __FUNCTION__, __VA_ARGS__);

#define CIDER_NOTIFY_CONSTRUCTOR(...)                                         \
  auto scopeGuard = cider::recorder::ActionsObservable::getInstance().notify( \
      __FUNCTION__, _impl.get(), __VA_ARGS__);

#define CIDER_NOTIFY_CONSTRUCTOR_NO_ARGS                                      \
  auto scopeGuard = cider::recorder::ActionsObservable::getInstance().notify( \
      __FUNCTION__, _impl.get());

#define CIDER_NOTIFY_ASSIGNMENT(PARAM)                                        \
  auto scopeGuard = cider::recorder::ActionsObservable::getInstance().notify( \
      _impl.get(), cider::recorder::BinaryOpType::Assignment, PARAM);

}  // namespace recorder
}  // namespace cider
