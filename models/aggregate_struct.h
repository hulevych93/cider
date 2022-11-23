#pragma once

namespace gunit {
namespace models {

// Aggregate structures have no constructor that we can hook, as well
// as we can't hook fields assignments after the object has been constructed.
// But when the object is used as parameter of the call we can obtain
// actual information from it.

struct Aggregate final {
  int number = 0;
  bool condition = false;
};

bool operator==(const Aggregate& l, const Aggregate& r);

// function to test user data as parameter and return value
Aggregate function_test_aggregate(const Aggregate& arg);

}  // namespace models
}  // namespace gunit
