#include "aggregate_struct.h"

namespace gunit {
namespace models {

Aggregate function_test_aggregate(const Aggregate& arg) {
  return arg;
}

Aggregate* function_test_aggregate(Aggregate* arg) {
  return arg;
}

bool Aggregate::operator==(const Aggregate& r) const {
  return condition == r.condition && number == r.number;
}

}  // namespace models
}  // namespace gunit
