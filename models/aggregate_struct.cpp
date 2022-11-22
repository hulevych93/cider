#include "aggregate_struct.h"

#include "recorder/actions_observer.h"
#include "recorder/details/params.h"
#include "recorder/details/user_data.h"

namespace gunit {
namespace recorder {

template <>
Argument generateImpl(const Aggregate& that, CodeSink& sink) {
  ParamVisitor visitor;
  std::string code("local aggr = example.Aggregate()\n");
  code += "aggr.condition = " + visitor(that.condition) + "\n";
  code += "aggr.number = " + visitor(that.number) + "\n";
  sink.addLocal("aggr", std::move(code));
  return Argument{"aggr"};
}

}  // namespace recorder
}  // namespace gunit

Aggregate function_test_aggregate(Aggregate arg) {
  GUNIT_NOTIFY_FREE_FUNCTION("function_test_aggregate({})", arg);
  return arg;
}

bool operator==(const Aggregate& l, const Aggregate& r) {
  return l.condition == r.condition && l.number == r.number;
}
