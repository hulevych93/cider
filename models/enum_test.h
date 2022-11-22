#pragma once

namespace gunit {
namespace models {

enum class SomeEnumeration { first_value = 0, second_value = 1 };

// function to test enum as parameter and return value
SomeEnumeration function_test_enumeration(SomeEnumeration arg);

}  // namespace models
}  // namespace gunit
