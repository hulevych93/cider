#include "derived_class_types.h"

namespace gunit {
namespace models {

SomeBase::SomeBase(const char* str) : _impl(str) {}

std::string SomeBase::sayHello() const { return _impl + ", hello!"; }

SomeDerived::SomeDerived(const char* str) : SomeBase(str), _impl(str) {}

std::string SomeDerived::sayGoodbye(int times) const {
    auto result = _impl;
    for (int i = 0; i < times; ++i)
      result += ", goodbye!";
    return result;
}

}  // namespace models
}  // namespace gunit
