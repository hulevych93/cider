#include "utils/test_suite.h"

#include "gunit-models/derived_class_types.h"

using namespace gunit::recorder;
using namespace gunit::models;
using namespace gunit::tests;
using namespace gunit;

struct DerivedClassTypesTest : TestSuite {};

const char* derived_class_types_test_script =
    R"(local object_1 = example.DerivedClass('false')
object_1:sayGoodbye(2)
)";

TEST_F(DerivedClassTypesTest, derived_class_types_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::DerivedClass object("false");
  object.sayGoodbye(2);

  SCOPED_TRACE("derived_class_types_test_script");
  testScript(derived_class_types_test_script, session);
}

const char* some_base_class_types_test_script =
    R"(local object_1 = example.BaseClass('true')
object_1:sayHello()
)";

TEST_F(DerivedClassTypesTest, some_base_class_types_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::BaseClass object("true");
  object.sayHello();

  SCOPED_TRACE("some_base_class_types_test_script");
  testScript(some_base_class_types_test_script, session);
}
