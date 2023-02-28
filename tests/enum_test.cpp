#include "utils/test_suite.h"

#include "gunit-models/enum.h"

using namespace gunit::recorder;
using namespace gunit::models;
using namespace gunit::tests;
using namespace gunit;

const char* function_test_enumeration_test_script =
    R"(example.function_test_enumeration(example.SomeEnumeration_second_value)
)";

TEST_F(TestSuite, function_test_enumeration_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  SomeEnumeration arg = SomeEnumeration::second_value;
  EXPECT_EQ(arg, gunit_hook::function_test_enumeration(arg));

  SCOPED_TRACE("function_test_enumeration_test_script");
  testScript(function_test_enumeration_test_script, session);
}
