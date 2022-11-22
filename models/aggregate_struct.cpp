#include "aggregate_struct.h"

#include "recorder/actions_observer.h"
#include "recorder/details/lua/lua_params.h"

namespace gunit {
namespace recorder {

template <>
std::string produceCode(const models::Aggregate& aggregate, CodeSink& sink) {
  ParamVisitor visitor;
  std::string code;
  code += "local {aggregate} = example.Aggregate();\n";
  code += "{aggregate}.condition = " + visitor(aggregate.condition) + "\n";
  code += "{aggregate}.number = " + visitor(aggregate.number) + "\n";
  return sink.processLocalVar("aggregate", std::move(code));
}

}  // namespace recorder

namespace models {

Aggregate function_test_aggregate(const Aggregate& arg) {
  GUNIT_NOTIFY_FREE_FUNCTION(arg);
  return arg;
}

bool operator==(const Aggregate& l, const Aggregate& r) {
  return l.condition == r.condition && l.number == r.number;
}

}  // namespace models
}  // namespace gunit
