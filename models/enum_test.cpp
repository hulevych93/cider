#include "enum_test.h"

#include "recorder/actions_observer.h"

namespace gunit {
namespace recorder {

template <>
std::string produceAggregateCode(const models::SomeEnumeration& that,
                                 CodeSink&) {
  switch (that) {
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

SomeEnumeration function_test_enumeration_impl(const SomeEnumeration arg) {
    return arg;
}

SomeEnumeration function_test_enumeration(const SomeEnumeration arg) {
    SomeEnumeration result;
    try {
        result = function_test_enumeration_impl(arg);
        GUNIT_NOTIFY_FREE_FUNCTION(result, arg);
        return result;
    }
    catch(const std::exception&) {
        // ex.what() notify
        throw;
    }
    catch(...) {
        // NOTIFY some expection
        throw;
    }
}

}  // namespace models
}  // namespace gunit
