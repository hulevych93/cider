// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "crossfile_types.h"

namespace cider {
namespace models {

Aggregate function_test_aggregate_in_other_file(const Aggregate& arg) {
  return arg;
}

FinalClass* function_test_class_construct_in_other_file(FinalClass* arg) {
  return new FinalClass{*arg};
}

}  // namespace models
}  // namespace cider
