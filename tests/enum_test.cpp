#include "utils/test_suite.h"

#include "enum.h"

using namespace cider::recorder;
using namespace cider::models;
using namespace cider::tests;
using namespace cider;

struct EnumTest : TestSuite {};

const char* function_test_enumeration_test_script =
    R"(example.function_test_enumeration(example.SomeEnumeration(example.SomeEnumeration_second_value))
)";

TEST_F(EnumTest, function_test_enumeration_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::SomeEnumeration arg = hook::SomeEnumeration::second_value;
  EXPECT_EQ(arg, hook::function_test_enumeration(arg));

  SCOPED_TRACE("function_test_enumeration_test_script");
  testScript(function_test_enumeration_test_script, session);
}
