#include "utils/test_suite.h"

#include "standart_types.h"

using namespace cider::recorder;
using namespace cider::models;
using namespace cider::tests;
using namespace cider;

struct StandardTypesTest : TestSuite {};

const char* string_struct_as_arg_test_script =
    R"(local object_1 = example.StringStruct()
object_1.field = 'test'
example.string_struct_as_arg(object_1)
)";

TEST_F(StandardTypesTest, string_struct_as_arg_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::StringStruct arg;
  arg.field = "test";
  EXPECT_EQ("test", hook::string_struct_as_arg(arg));

  SCOPED_TRACE("string_struct_as_arg_test");
  testScript(string_struct_as_arg_test_script, session);
}

const char* uint16_from_32_test_script =
    R"(example.uint16_from_32(example.Int(234))
)";

TEST_F(StandardTypesTest, uint16_from_32_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  std::uint32_t arg = 234;
  EXPECT_EQ(234, hook::uint16_from_32(arg));

  SCOPED_TRACE("uint16_from_32_test");
  testScript(uint16_from_32_test_script, session);
}
