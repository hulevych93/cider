#include "free_functions.h"

#include "recorder/actions_observer.h"

using namespace gunit::recorder;

int calculate_factorial(int value) {
  GUNIT_NOTIFY_FREE_FUNCTION("calculate_factorial({})", value);
  return !value ? 1 : value * calculate_factorial(value - 1);
}

bool is_this_sparta_word(const char* value) {
  GUNIT_NOTIFY_FREE_FUNCTION("is_this_sparta_word({})", value);
  return strcmp(value, "sparta") == 0;
}
