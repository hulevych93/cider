#include "aggregate_struct.h"

#include "recorder/actions_observer.h"
#include "recorder/details/lua/lua_params.h"

namespace gunit {
namespace recorder {

template <>
std::string produceAggregateCode(const models::Aggregate& aggregate,
                                 CodeSink& sink) {
  lua::ParamVisitor visitor;
  std::string code;
  code += "local {var} = example.Aggregate()\n";
  code += "{var}.condition = " + visitor(aggregate.condition) + "\n";
  code += "{var}.number = " + visitor(aggregate.number) + "\n";
  return sink.processLocalVar(std::move(code));
}

} // namespace recorder

namespace models {
namespace generated {

Aggregate function_test_aggregate(const Aggregate& arg) {
  try {
    auto result = models::function_test_aggregate(arg);
    GUNIT_NOTIFY_FREE_FUNCTION(result, arg);
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

Aggregate* function_test_aggregate(Aggregate* arg) {
  try {
    Aggregate* result = models::function_test_aggregate(arg);
    GUNIT_NOTIFY_FREE_FUNCTION(result, arg);
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

} // namespace generated
} // namespace models
} // namespace gunit
