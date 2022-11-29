#include "derived_class_types.h"

#include "recorder/actions_observer.h"

namespace gunit {
namespace models {

class SomeBaseImpl {
 public:
  SomeBaseImpl(const char* str) : _impl(str) {}

  std::string sayHello() { return _impl + ", hello!"; }

 private:
  std::string _impl;
};

class SomeDerivedImpl : public SomeBaseImpl {
 public:
  SomeDerivedImpl(const char* str) : SomeBaseImpl(str), _impl(str) {}

  std::string sayGoodbye(int times) const {
    auto result = _impl;
    for (int i = 0; i < times; ++i)
      result += ", goodbye!";
    return result;
  }

 private:
  std::string _impl;
};

SomeBase::SomeBase(std::shared_ptr<SomeBaseImpl> impl) : _impl(impl) {}

void SomeBase::setImpl(std::shared_ptr<SomeBaseImpl> impl) {
  _impl = impl;
}

SomeBase::SomeBase(const char* str) {
  _impl = std::make_shared<SomeBaseImpl>(str);
  GUNIT_NOTIFY_CONSTRUCTOR(str);
}

std::string SomeBase::sayHello() const {
  const auto result = _impl->sayHello();
  GUNIT_NOTIFY_METHOD_NO_ARGS(result);
  return result;
}

SomeDerived::SomeDerived(const char* str)
    : SomeBase(std::shared_ptr<SomeBaseImpl>()) {
  _impl = std::make_shared<SomeDerivedImpl>(str);
  SomeBase::setImpl(_impl);
  GUNIT_NOTIFY_CONSTRUCTOR(str);
}

std::string SomeDerived::sayGoodbye(const int times) const {
  const auto result = _impl->sayGoodbye(times);
  GUNIT_NOTIFY_METHOD(result, times);
  return result;
}

SomeDerived::SomeDerived(std::shared_ptr<SomeDerivedImpl> impl)
    : SomeBase(impl), _impl(impl) {}

void SomeDerived::setImpl(std::shared_ptr<SomeDerivedImpl> impl) {
  _impl = impl;
}

}  // namespace models
}  // namespace gunit
