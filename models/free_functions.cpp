#include "free_functions.h"

#include <string.h>

namespace gunit {
namespace models {

void some_function_no_return(bool) {}

int calculate_factorial(int value) {
  return !value ? 1 : value * calculate_factorial(value - 1);
}

bool is_this_sparta_word(const char* value) {
  return strcmp(value, "sparta") == 0;
}

int summ_these_two_params(int first, unsigned long second) {
  return first + second;
}

}  // namespace models
}  // namespace gunit
