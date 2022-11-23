#include "class_construct.h"

#include "recorder/actions_observer.h"
#include "recorder/details/lua/lua_params.h"

namespace gunit {
namespace models {

ClassConstruct::ClassConstruct() : _number(0), _condition(false) {
  GUNIT_NOTIFY_CONSTRUCTOR("ClassConstruct");
}

ClassConstruct::ClassConstruct(int number, bool condition)
    : _number(number), _condition(condition) {
  GUNIT_NOTIFY_CONSTRUCTOR("ClassConstruct", number, condition);
}

void ClassConstruct::someMethod(const int newNumber) {
  GUNIT_NOTIFY_METHOD_CALL(newNumber);
  _number = newNumber;
}

bool ClassConstruct::operator==(const ClassConstruct& that) {
  return _condition == that._condition && _number == that._number;
}

ClassConstruct function_test_class_construct(const ClassConstruct& arg) {
  GUNIT_NOTIFY_FREE_FUNCTION(arg);
  return arg;
}

}  // namespace models
}  // namespace gunit
