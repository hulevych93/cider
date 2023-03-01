#include "final_class.h"

namespace gunit {
namespace models {

FinalClass::FinalClass() : _number(0), _condition(false) {}
FinalClass::FinalClass(int number, bool condition)
    : _number(number), _condition(condition) {}

FinalClass::FinalClass(const FinalClass& other)
    : _number(other._number), _condition(other._condition) {}
FinalClass::FinalClass(FinalClass&& other)
    : _number(other._number), _condition(other._condition) {}

void FinalClass::someMethod(int newNumber) {
  _number = newNumber;
}

FinalClass& FinalClass::operator=(const FinalClass& other) {
  _number = other._number;
  _condition = other._condition;
  return *this;
}

FinalClass& FinalClass::operator=(FinalClass&& other) {
  _number = other._number;
  _condition = other._condition;
  return *this;
}

bool FinalClass::operator==(const FinalClass& that) const {
  return _condition == that._condition && _number == that._number;
}
bool FinalClass::operator!=(const FinalClass& that) const {
  return (*this) != that;
}

FinalClass function_make_class_construct_obj() {
  return FinalClass{4351, true};
}

FinalClass* function_make_class_construct_obj_ptr() {
  return new FinalClass{411, true};
}

FinalClass function_test_class_construct(const FinalClass& arg) {
  return arg;
}

FinalClass* function_test_class_construct(FinalClass* arg) {
  return arg;
}

}  // namespace models
}  // namespace gunit
