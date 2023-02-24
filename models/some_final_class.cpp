#include "some_final_class.h"

namespace gunit {
namespace models {

SomeFinalClass::SomeFinalClass() : _number(0), _condition(false) {}
SomeFinalClass::SomeFinalClass(int number, bool condition)
    : _number(number), _condition(condition) {}
SomeFinalClass::SomeFinalClass(const SomeFinalClass& other)
    : _number(other._number), _condition(other._condition) {}
SomeFinalClass::SomeFinalClass(SomeFinalClass&& other)
    : _number(other._number), _condition(other._condition) {}

void SomeFinalClass::someMethod(int newNumber) {
  _number = newNumber;
}

SomeFinalClass& SomeFinalClass::operator=(const SomeFinalClass& other) {
  _number = other._number;
  _condition = other._condition;
  return *this;
}
SomeFinalClass& SomeFinalClass::operator=(SomeFinalClass&& other) {
  _number = other._number;
  _condition = other._condition;
  return *this;
}

bool SomeFinalClass::operator==(const SomeFinalClass& that) const {
  return _condition == that._condition && _number == that._number;
}
bool SomeFinalClass::operator!=(const SomeFinalClass& that) const {
  return (*this) != that;
}

SomeFinalClass function_make_class_construct_obj() {
  return SomeFinalClass{4351, true};
}

SomeFinalClass* function_make_class_construct_obj_ptr() {
  return new SomeFinalClass{411, true};
}

SomeFinalClass function_test_class_construct(const SomeFinalClass& arg) {
  return arg;
}

SomeFinalClass* function_test_class_construct(SomeFinalClass* arg) {
  return arg;
}

}  // namespace models
}  // namespace gunit
