#include "polymorphic_types.h"

#include "recorder/actions_observer.h"

namespace gunit {
namespace models {
namespace generated {

SomeInterface::SomeInterface(std::shared_ptr<models::SomeInterface> impl)
    : _impl(impl) {}
void SomeInterface::setImpl(std::shared_ptr<models::SomeInterface> impl) {
  _impl = impl;
}

bool SomeInterface::isEmpty() const {
  const auto result = _impl->isEmpty();
  GUNIT_NOTIFY_METHOD_NO_ARGS(result);
  return result;
}

StringInterface::StringInterface(std::shared_ptr<models::StringInterface> impl)
    : SomeInterface(impl), _impl(impl) {}
void StringInterface::setImpl(std::shared_ptr<models::StringInterface> impl) {
  _impl = impl;
}

StringInterface::StringInterface(const char* str) {
  _impl = std::make_shared<models::StringInterface>(str);
  GUNIT_NOTIFY_CONSTRUCTOR(str);
}
bool StringInterface::isEmpty() const {
  const auto result = _impl->isEmpty();
  GUNIT_NOTIFY_METHOD_NO_ARGS(result);
  return result;
}

OtherStringInterface::OtherStringInterface(const bool empty)
    : StringInterface(std::shared_ptr<models::StringInterface>()) {
  _impl = std::make_shared<models::OtherStringInterface>(empty);
  StringInterface::setImpl(_impl);
  GUNIT_NOTIFY_CONSTRUCTOR(empty);
}

OtherStringInterface::OtherStringInterface(
    std::shared_ptr<models::OtherStringInterface> impl)
    : StringInterface(impl), _impl(impl) {}
void OtherStringInterface::setImpl(
    std::shared_ptr<models::OtherStringInterface> impl) {
  _impl = impl;
}

SomeInterface* makeSomeInterface(const char* str) {
  auto impl =
      std::shared_ptr<models::SomeInterface>(models::makeSomeInterface(str));
  auto result = new SomeInterface(impl);
  GUNIT_NOTIFY_FREE_FUNCTION(impl.get(), str);
  return result;
}

}  // namespace generated
}  // namespace models
}  // namespace gunit
