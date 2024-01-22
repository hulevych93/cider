#pragma once

#include "aggregate_struct.h"

namespace cider {
namespace models {

struct AggregateDerived final : Aggregate {
  float floatingNumber;
};

static_assert (std::is_aggregate_v<AggregateDerived>, "is_aggregate_v AggregateDerived");

AggregateDerived function_test_aggregate_derived(const AggregateDerived& arg);
AggregateDerived* function_test_aggregate_derived(AggregateDerived* arg);

}  // namespace models
}  // namespace cider
