#include "free_functions.h"

#include "recorder/actions_observer.h"

namespace gunit {

namespace recorder {

std::string getFreeFunctionTemplate() {
  return "example.";
}

}  // namespace recorder

namespace models {

using namespace gunit::recorder;

int calculate_factorial(int value) {
  GUNIT_NOTIFY_FREE_FUNCTION(value);
  return !value ? 1 : value * calculate_factorial(value - 1);
}

bool is_this_sparta_word(const char* value) {
  GUNIT_NOTIFY_FREE_FUNCTION(value);
  return strcmp(value, "sparta") == 0;
}

int summ_these_two_params(int first, unsigned long second) {
  GUNIT_NOTIFY_FREE_FUNCTION(first, second);
  return first + second;
}

}  // namespace models
}  // namespace gunit
