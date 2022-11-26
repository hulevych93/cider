#pragma once

namespace gunit {
namespace models {

// Function with no return
void some_function_no_return(bool);

// Recurring free function that calculates factorial
int calculate_factorial(int value);

// Returns true if passed "sparta" string
bool is_this_sparta_word(const char* value);

// Two parameters function testing
int summ_these_two_params(int first, unsigned long second);

}  // namespace models
}  // namespace gunit
