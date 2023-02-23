#include "derived_class_types.h"

#include "recorder/actions_observer.h"

namespace gunit {
namespace models {
namespace generated {

SomeBase::SomeBase(std::shared_ptr<models::SomeBase> impl) : _impl(impl) {}

void SomeBase::setImpl(std::shared_ptr<models::SomeBase> impl) {
  _impl = impl;
}

SomeBase::SomeBase(const char* str) {
  _impl = std::make_shared<models::SomeBase>(str);
  GUNIT_NOTIFY_CONSTRUCTOR(str);
}

std::string SomeBase::sayHello() const {
  const auto result = _impl->sayHello();
  GUNIT_NOTIFY_METHOD_NO_ARGS(result);
  return result;
}

SomeDerived::SomeDerived(const char* str)
    : SomeBase(std::shared_ptr<models::SomeBase>()) {
  _impl = std::make_shared<models::SomeDerived>(str);
  SomeBase::setImpl(_impl);
  GUNIT_NOTIFY_CONSTRUCTOR(str);
}

std::string SomeDerived::sayGoodbye(const int times) const {
  const auto result = _impl->sayGoodbye(times);
  GUNIT_NOTIFY_METHOD(result, times);
  return result;
}

SomeDerived::SomeDerived(std::shared_ptr<models::SomeDerived> impl)
    : SomeBase(impl), _impl(impl) {}

void SomeDerived::setImpl(std::shared_ptr<models::SomeDerived> impl) {
  _impl = impl;
}

} // namespace generated
} // namespace models
} // namespace gunit
