#pragma once

#include "aggregate_struct.h"

namespace gunit {
namespace models {

struct AggregateDerived final : Aggregate {
  float floatingNumber;
};

AggregateDerived function_test_aggregate_derived(const AggregateDerived& arg);
AggregateDerived* function_test_aggregate_derived(AggregateDerived* arg);

}  // namespace models
}  // namespace gunit
