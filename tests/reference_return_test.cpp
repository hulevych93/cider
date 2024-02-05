#include "utils/test_suite.h"

#include "reference_return.h"

using namespace cider::recorder;
using namespace cider::models;
using namespace cider::tests;
using namespace cider;

struct ReferenceReturnTest : TestSuite {};

const char* copy_and_inc_test_script =
    R"(local object_1 = example.RefStruct()
local object_2 = object_1:copy()
object_1:inc()
object_2:inc()
)";

TEST_F(ReferenceReturnTest, copy_and_inc_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::RefStruct str;
  auto str1 = str.copy();

  EXPECT_EQ(2, str.inc());
  EXPECT_EQ(2, str1.inc());

  SCOPED_TRACE("copy_and_inc_test_script");
  testScript(copy_and_inc_test_script, session);
}

const char* ref_and_inc_test_script =
    R"(local object_1 = example.RefStruct()
object_1 = object_1:self()
object_1:inc()
object_1:inc()
)";

TEST_F(ReferenceReturnTest, ref_and_inc_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::RefStruct str;
  auto str1 = str.self();

  EXPECT_EQ(2, str.inc());
  EXPECT_EQ(3, str1.inc());

  SCOPED_TRACE("ref_and_inc_test_script");
  testScript(ref_and_inc_test_script, session);
}
