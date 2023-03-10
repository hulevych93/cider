#include "utils/test_suite.h"

#include "gunit-models/crossfile_types.h"

using namespace gunit::recorder;
using namespace gunit::models;
using namespace gunit::tests;
using namespace gunit;

struct CrossfileTest : TestSuite {};

const char* function_test_aggregate_in_other_file_test_script =
    R"(local object_1 = example.Aggregate()
object_1.condition = false
object_1.number = 0
example.function_test_aggregate_in_other_file(object_1)
)";

TEST_F(CrossfileTest, function_test_aggregate_in_other_file_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::Aggregate object;
  hook::function_test_aggregate_in_other_file(object);

  SCOPED_TRACE("function_test_aggregate_in_other_file_test_script");
  testScript(function_test_aggregate_in_other_file_test_script, session);
}

const char* function_test_class_construct_in_other_file_test_script =
    R"(local object_1 = example.FinalClass()
local object_2 = example.function_test_class_construct_in_other_file(object_1)
)";

TEST_F(CrossfileTest, function_test_class_construct_in_other_file_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::FinalClass object;
  hook::function_test_class_construct_in_other_file(&object);

  SCOPED_TRACE("function_test_class_construct_in_other_file_test_script");
  testScript(function_test_class_construct_in_other_file_test_script, session);
}