#include "utils/test_suite.h"

#include "gunit-models/aggregate_struct.h"

using namespace gunit::recorder;
using namespace gunit::models;
using namespace gunit::tests;
using namespace gunit;

struct AggregateStructTest : TestSuite {};

static const char* function_test_aggregate_test_script =
    R"(local object_1 = example.Aggregate()
object_1.condition = true
object_1.number = 10
example.function_test_aggregate(object_1)
)";

TEST_F(AggregateStructTest, function_test_aggregate_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::Aggregate aggregateStruct{10, true};
  gunit_hook::function_test_aggregate(aggregateStruct);

  SCOPED_TRACE("function_test_aggregate_test_script");
  testScript(function_test_aggregate_test_script, session);
}

TEST_F(AggregateStructTest, function_test_aggregate_ptr_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::Aggregate aggregateStruct{10, true};
  gunit_hook::function_test_aggregate(&aggregateStruct);

  SCOPED_TRACE("function_test_aggregate_test_script");
  testScript(function_test_aggregate_test_script, session);
}
