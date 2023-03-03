#include "utils/test_suite.h"

#include "gunit-models/aggregate_struct.h"
#include "gunit-models/derived_agregate_types.h"

using namespace gunit::recorder;
using namespace gunit::models;
using namespace gunit::tests;
using namespace gunit;

struct DerivedAggregateTypesTest : TestSuite {};

static const char* function_test_aggregate_test_script =
    R"(local object_1 = example.Aggregate()
object_1.condition = true
object_1.number = 10
example.function_test_aggregate(object_1)
)";

TEST_F(DerivedAggregateTypesTest, function_test_aggregate_with_derived_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::AggregateDerived aggregateStruct{10, true, 0.5};
  hook::function_test_aggregate(aggregateStruct);

  SCOPED_TRACE("function_test_aggregate_test_script");
  testScript(function_test_aggregate_test_script, session);
}

TEST_F(DerivedAggregateTypesTest,
       function_test_aggregate_ptr_with_derived_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::AggregateDerived aggregateStruct{10, true, 0.5};
  hook::function_test_aggregate(&aggregateStruct);

  SCOPED_TRACE("function_test_aggregate_test_script");
  testScript(function_test_aggregate_test_script, session);
}

const char* function_test_aggregate_derived_test_script =
    R"(local object_1 = example.AggregateDerived()
object_1.condition = true
object_1.number = 10
object_1.floatingNumber = 0.5
example.function_test_aggregate_derived(object_1)
)";

TEST_F(DerivedAggregateTypesTest, function_test_aggregate_derived_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::AggregateDerived aggregateStruct{10, true, 0.5};
  hook::function_test_aggregate_derived(aggregateStruct);

  SCOPED_TRACE("function_test_aggregate_derived_test_script");
  testScript(function_test_aggregate_derived_test_script, session);
}

TEST_F(DerivedAggregateTypesTest, function_test_aggregate_derived_ptr_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::AggregateDerived aggregateStruct{10, true, 0.5};
  hook::function_test_aggregate_derived(&aggregateStruct);

  SCOPED_TRACE("function_test_aggregate_derived_test_script");
  testScript(function_test_aggregate_derived_test_script, session);
}
