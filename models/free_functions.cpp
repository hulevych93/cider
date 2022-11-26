#include "free_functions.h"

#include "recorder/actions_observer.h"

namespace gunit {
namespace models {

using namespace gunit::recorder;

namespace {

void some_function_no_return_impl(bool) {}

int calculate_factorial_impl(int value) {
  return !value ? 1 : value * calculate_factorial_impl(value - 1);
}

bool is_this_sparta_word_impl(const char* value) {
  return strcmp(value, "sparta") == 0;
}

int summ_these_two_params_impl(int first, unsigned long second) {
  return first + second;
}

}  // namespace

void some_function_no_return(bool some) {
  try {
    some_function_no_return_impl(some);
    GUNIT_NOTIFY_FREE_FUNCTION_NO_RETURN(some);
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

int calculate_factorial(int value) {
  try {
    auto result = calculate_factorial_impl(value);
    GUNIT_NOTIFY_FREE_FUNCTION(result, value);
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

bool is_this_sparta_word(const char* value) {
  try {
    auto result = is_this_sparta_word_impl(value);
    GUNIT_NOTIFY_FREE_FUNCTION(result, value);
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

int summ_these_two_params(int first, unsigned long second) {
  try {
    auto result = summ_these_two_params_impl(first, second);
    GUNIT_NOTIFY_FREE_FUNCTION(result, first, second);
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

}  // namespace models
}  // namespace gunit
