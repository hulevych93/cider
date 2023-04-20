#pragma once

namespace cider {
namespace models {

// Function with no return and no args
void some_function_no_return_no_args();

// Function with no return
void some_function_no_return(bool arg);

// Recurring free function that calculates factorial
int calculate_factorial(int value);

// Returns true if passed "sparta" string
bool is_this_sparta_word(const char* value);

// Two parameters function testing
int summ_these_two_params(int first, unsigned long second);

}  // namespace models
}  // namespace cider
