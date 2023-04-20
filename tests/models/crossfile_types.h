#pragma once

#include "aggregate_struct.h"
#include "final_class.h"

namespace cider {
namespace models {

Aggregate function_test_aggregate_in_other_file(const Aggregate& arg);
FinalClass* function_test_class_construct_in_other_file(FinalClass* arg);

}  // namespace models
}  // namespace cider
