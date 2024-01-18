#pragma once

namespace cider {
namespace models {

enum class SomeEnumeration : unsigned char {
  first_value = 0,
  second_value,
  third_value
};

// function to test enum as parameter and return value
SomeEnumeration function_test_enumeration(SomeEnumeration arg);

}  // namespace models
}  // namespace cider
