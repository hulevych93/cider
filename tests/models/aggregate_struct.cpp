// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "aggregate_struct.h"

namespace cider {
namespace models {

Aggregate function_test_aggregate(const Aggregate& arg) {
  return arg;
}

Aggregate* function_test_aggregate(Aggregate* arg) {
  return arg;
}

}  // namespace models
}  // namespace cider
