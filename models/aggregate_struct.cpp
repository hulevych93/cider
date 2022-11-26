#include "aggregate_struct.h"

#include "recorder/actions_observer.h"
#include "recorder/details/lua/lua_params.h"

namespace gunit {
namespace recorder {

template <>
std::string produceAggregateCode(const models::Aggregate& aggregate,
                                 CodeSink& sink) {
  ParamVisitor visitor;
  std::string code;
  code += "local {var} = example.Aggregate()\n";
  code += "{var}.condition = " + visitor(aggregate.condition) + "\n";
  code += "{var}.number = " + visitor(aggregate.number) + "\n";
  return sink.processLocalVar(std::move(code));
}

}  // namespace recorder

namespace models {

Aggregate function_test_aggregate(const Aggregate& arg) {
  GUNIT_NOTIFY_FREE_FUNCTION(arg, arg);
  return arg;
}

Aggregate* function_test_aggregate_ptr(Aggregate* arg) {
  return arg;
}

bool Aggregate::operator==(const Aggregate& r) const {
  return condition == r.condition && number == r.number;
}

}  // namespace models
}  // namespace gunit
