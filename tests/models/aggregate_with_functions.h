#pragma once

#include <cstdint>
#include <string>

namespace cider {
namespace models {

struct AggregateWithFunctionsStruct final {
  int getField() { return field; }

  int field;
};

int test_aggregate_with_functions(AggregateWithFunctionsStruct& arg);

static_assert (std::is_pod_v<AggregateWithFunctionsStruct>, "pod SomeRefStruct");

}  // namespace models
}  // namespace cider
