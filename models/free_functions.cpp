#include "free_functions.h"

#include "recorder/actions_observer.h"

namespace gunit {

namespace recorder {

std::string getFunctionTemplate() {
  return "example.";
}

}  // namespace recorder

namespace models {

using namespace gunit::recorder;

void some_function_no_return(bool some)
{
    GUNIT_NOTIFY_FREE_FUNCTION_NO_RETURN(some);
}

int calculate_factorial_impl(int value) {
    return !value ? 1 : value * calculate_factorial_impl(value - 1);
}

int calculate_factorial(int value) {
  const auto result = calculate_factorial_impl(value);
      GUNIT_NOTIFY_FREE_FUNCTION(result, value)
      return result;
}

bool is_this_sparta_word(const char* value) {
  const auto result = strcmp(value, "sparta") == 0;
  GUNIT_NOTIFY_FREE_FUNCTION(result, value);
  return result;
}

int summ_these_two_params(int first, unsigned long second) {
  const auto result =  first + second;
  GUNIT_NOTIFY_FREE_FUNCTION(result, first, second);
  return result;
}

}  // namespace models
}  // namespace gunit
