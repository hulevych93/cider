// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "derived_agregate_types.h"

namespace cider {
namespace models {

AggregateDerived function_test_aggregate_derived(const AggregateDerived& arg) {
  return arg;
}

AggregateDerived* function_test_aggregate_derived(AggregateDerived* arg) {
  return arg;
}

}  // namespace models
}  // namespace cider
