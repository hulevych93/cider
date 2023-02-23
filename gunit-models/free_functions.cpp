#include "free_functions.h"

#include "recorder/actions_observer.h"

namespace gunit {
namespace models {
namespace generated {

void some_function_no_return(bool some) {
  try {
    models::some_function_no_return(some);
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
    auto result = models::calculate_factorial(value);
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
    auto result = models::is_this_sparta_word(value);
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
    auto result = models::summ_these_two_params(first, second);
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

} // namespace generated
} // namespace models
} // namespace gunit
