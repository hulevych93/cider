#include "derived_agregate_types.h"

#include "recorder/actions_observer.h"
#include "recorder/details/lua/lua_params.h"

namespace gunit {
namespace recorder {

template <>
std::string produceAggregateCode(const models::AggregateDerived& aggregate,
                                 CodeSink& sink) {
  lua::ParamVisitor visitor;
  std::string code;
  code += "local {var} = example.AggregateDerived()\n";
  code += "{var}.condition = " + visitor(aggregate.condition) + "\n";
  code += "{var}.number = " + visitor(aggregate.number) + "\n";
  code += "{var}.floatingNumber = " + visitor(aggregate.floatingNumber) + "\n";
  return sink.processLocalVar(std::move(code));
}

}  // namespace recorder

namespace models {
namespace generated {

AggregateDerived function_test_aggregate_derived(const AggregateDerived& arg) {
  try {
    auto result = models::function_test_aggregate_derived(arg);
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

AggregateDerived* function_test_aggregate_derived(AggregateDerived* arg) try {
  auto result = models::function_test_aggregate_derived(arg);
  GUNIT_NOTIFY_FREE_FUNCTION(result, arg);
  return result;
} catch (const std::exception&) {
  // ex.what() notify
  throw;
} catch (...) {
  // NOTIFY some expection
  throw;
}

}  // namespace generated
}  // namespace models
}  // namespace gunit
