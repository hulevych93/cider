#include "derived_class_types.h"

namespace cider {
namespace models {

BaseClass::BaseClass(const char* str) : _impl(str) {}

std::string BaseClass::sayHello() const {
  return _impl + ", hello!";
}

DerivedClass::DerivedClass(const char* str) : BaseClass(str), _impl(str) {}

std::string DerivedClass::sayGoodbye(int times) const {
  auto result = _impl;
  for (int i = 0; i < times; ++i)
    result += ", goodbye!";
  return result;
}

}  // namespace models
}  // namespace cider
