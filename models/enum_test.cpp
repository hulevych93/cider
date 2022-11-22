#include "enum_test.h"

#include "recorder/actions_observer.h"

namespace gunit {

namespace recorder {

template <>
Argument generateImpl(const models::SomeEnumeration& that, CodeSink&) {
    switch(that) {
    case models::SomeEnumeration::first_value:
        return "example.SomeEnumeration_first_value";
        break;
    case models::SomeEnumeration::second_value:
        return "example.SomeEnumeration_second_value";
        break;
    }
    return {};
}

}  // namespace recorder

namespace models {

SomeEnumeration function_test_enumeration(const SomeEnumeration arg) {
  GUNIT_NOTIFY_FREE_FUNCTION("function_test_enumeration({})", arg);
  return arg;
}

}  // namespace models
}  // namespace gunit
