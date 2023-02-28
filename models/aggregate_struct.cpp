#include "aggregate_struct.h"

namespace gunit {
namespace models {

Aggregate function_test_aggregate(const Aggregate& arg) {
  return arg;
}

Aggregate* function_test_aggregate(Aggregate* arg) {
  return arg;
}

}  // namespace models
}  // namespace gunit
