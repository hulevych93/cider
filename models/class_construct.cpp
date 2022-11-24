#include "class_construct.h"

#include "recorder/actions_observer.h"
#include "recorder/details/lua/lua_params.h"

namespace gunit {
namespace models {

ClassConstruct::ClassConstruct() : _number(0), _condition(false) {
  GUNIT_NOTIFY_CONSTRUCTOR("ClassConstruct");
}

ClassConstruct::ClassConstruct(const ClassConstruct& other)
    : _number(other._number), _condition(other._condition) {
  GUNIT_NOTIFY_CONSTRUCTOR("ClassConstruct", other);
}

ClassConstruct::ClassConstruct(ClassConstruct&& other)
    : _number(other._number), _condition(other._condition) {
  GUNIT_NOTIFY_CONSTRUCTOR("ClassConstruct", other);
}

ClassConstruct::ClassConstruct(int number, bool condition)
    : _number(number), _condition(condition) {
  GUNIT_NOTIFY_CONSTRUCTOR("ClassConstruct", number, condition);
}

void ClassConstruct::someMethod(const int newNumber) {
  GUNIT_NOTIFY_METHOD(newNumber);
  _number = newNumber;
}

ClassConstruct& ClassConstruct::operator=(const ClassConstruct& other) {
  GUNIT_NOTIFY_ASSIGNMENT(other);
  _number = other._number;
  _condition = other._condition;
  return *this;
}

ClassConstruct& ClassConstruct::operator=(ClassConstruct&& other) {
  GUNIT_NOTIFY_ASSIGNMENT(other);
  _number = other._number;
  _condition = other._condition;
  return *this;
}

bool ClassConstruct::operator==(const ClassConstruct& that) const {
  return _condition == that._condition && _number == that._number;
}

bool ClassConstruct::operator!=(const ClassConstruct& that) const {
  return (*this) != that;
}

ClassConstruct function_test_class_construct(const ClassConstruct& arg) {
  GUNIT_NOTIFY_FREE_FUNCTION(arg);
  return arg;
}

}  // namespace models
}  // namespace gunit
