#include "utils/test_suite.h"

#include "aggregate_struct.h"

using namespace cider::recorder;
using namespace cider::models;
using namespace cider::tests;
using namespace cider;

struct AggregateStructTest : TestSuite {};

static const char* function_test_aggregate_test_script =
    R"(local object_1 = example.Aggregate()
object_1.number = example.Int(10)
object_1.condition = true
example.function_test_aggregate(object_1)
)";

TEST_F(AggregateStructTest, function_test_aggregate_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::Aggregate aggregateStruct{10, true};
  hook::function_test_aggregate(aggregateStruct);

  SCOPED_TRACE("function_test_aggregate_test_script");
  testScript(function_test_aggregate_test_script, session);
}

TEST_F(AggregateStructTest, function_test_aggregate_ptr_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::Aggregate aggregateStruct{10, true};
  hook::function_test_aggregate(&aggregateStruct);

  SCOPED_TRACE("function_test_aggregate_test_script");
  testScript(function_test_aggregate_test_script, session);
}
