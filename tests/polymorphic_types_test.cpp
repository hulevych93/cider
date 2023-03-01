#include "utils/test_suite.h"

#include "gunit-models/polymorphic_types.h"

using namespace gunit::recorder;
using namespace gunit::models;
using namespace gunit::tests;
using namespace gunit;

const char* some_interface_test_script =
    R"(local object_1 = example.StringInterface('abc')
object_1:isEmpty()
)";

TEST_F(TestSuite, some_interface_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::StringInterface object{"abc"};
  object.isEmpty();

  SCOPED_TRACE("some_interface_test_script");
  testScript(some_interface_test_script, session);
}

TEST_F(TestSuite, some_interface_test_ptr) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::StringInterface* object = new gunit_hook::StringInterface{"abc"};
  object->isEmpty();
  delete object;

  SCOPED_TRACE("some_interface_test_script");
  testScript(some_interface_test_script, session);
}

const char* some_other_interface_test_script =
    R"(local object_1 = example.OtherStringInterface(false)
object_1:isEmpty()
)";

TEST_F(TestSuite, DISABLED_some_other_interface_test_ptr) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::StringInterface* object =
      new gunit_hook::OtherStringInterface{false};
  object->isEmpty();
  delete object;

  SCOPED_TRACE("some_other_interface_test_script");
  testScript(some_other_interface_test_script, session);
}

const char* make_some_interface_test_script =
    R"(local object_1 = example.makeSomeInterface('false')
object_1:isEmpty()
)";

TEST_F(TestSuite, make_some_interface_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::SomeInterface* object = gunit_hook::makeSomeInterface("false");
  object->isEmpty();
  delete object;

  SCOPED_TRACE("make_some_interface_test_script");
  testScript(make_some_interface_test_script, session);
}
