#include "polymorphic_types.h"

#include "recorder/actions_observer.h"

namespace gunit {
namespace models {

class StringInterfaceImpl : public SomeInterface {
 public:
  StringInterfaceImpl(const char* str) : _impl(str) {}
  bool isEmpty() const override { return _impl.empty(); }

 private:
  std::string _impl;
};

class OtherStringInterfaceImpl final : public StringInterfaceImpl {
 public:
  OtherStringInterfaceImpl(bool empty)
      : StringInterfaceImpl(empty ? "" : "abc") {}
};

StringInterface::StringInterface(std::shared_ptr<StringInterfaceImpl> impl)
    : _impl(impl) {}
void StringInterface::setImpl(std::shared_ptr<StringInterfaceImpl> impl) {
  _impl = impl;
}

StringInterface::StringInterface(const char* str) {
  _impl = std::make_shared<StringInterfaceImpl>(str);
  GUNIT_NOTIFY_CONSTRUCTOR(str);
}
bool StringInterface::isEmpty() const {
  const auto result = _impl->isEmpty();
  GUNIT_NOTIFY_METHOD_NO_ARGS(result);
  return result;
}

OtherStringInterface::OtherStringInterface(const bool empty)
    : StringInterface(std::shared_ptr<StringInterfaceImpl>()) {
  _impl = std::make_shared<OtherStringInterfaceImpl>(empty);
  StringInterface::setImpl(_impl);
  GUNIT_NOTIFY_CONSTRUCTOR(empty);
}

OtherStringInterface::OtherStringInterface(
    std::shared_ptr<OtherStringInterfaceImpl> impl)
    : StringInterface(impl), _impl(impl) {}
void OtherStringInterface::setImpl(
    std::shared_ptr<OtherStringInterfaceImpl> impl) {
  _impl = impl;
}

SomeInterface* makeSomeInterface(const char* str) {
    auto impl = std::make_shared<StringInterfaceImpl>(str);
    auto result = new StringInterface(impl);
    GUNIT_NOTIFY_FREE_FUNCTION(result, str);
    return result;
}

}  // namespace models
}  // namespace gunit
