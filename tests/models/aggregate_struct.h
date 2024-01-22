#pragma once

#include <type_traits>

namespace cider {
namespace models {

struct Aggregate {
  int number = 0;
  bool condition = false;
};

static_assert (std::is_aggregate_v<Aggregate>, "is_aggregate_v Aggregate");
static_assert (!std::is_pod_v<Aggregate>, "!is_pod_v Aggregate");

// function to test user data as parameter and return value
Aggregate function_test_aggregate(const Aggregate& arg);
Aggregate* function_test_aggregate(Aggregate* arg);

}  // namespace models
}  // namespace cider
