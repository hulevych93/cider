#include "utils/test_suite.h"

#include "gunit-models/final_class.h"

using namespace gunit::recorder;
using namespace gunit::models;
using namespace gunit::tests;
using namespace gunit;

TEST_F(TestSuite, class_construct_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::FinalClass object1;
  EXPECT_EQ("local object_1 = example.FinalClass()\n", session->getScript());

  gunit_hook::FinalClass object2(125, true);
  EXPECT_EQ("local object_1 = example.FinalClass(125, true)\n",
            session->getScript());
}

const char* class_method_test_script =
    R"(local object_1 = example.FinalClass()
object_1:someMethod(129)
)";

TEST_F(TestSuite, class_method_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::FinalClass object1;
  object1.someMethod(129);

  SCOPED_TRACE("class_method_test_script");
  testScript(class_method_test_script, session);
}

const char* function_test_class_construct_test_script =
    R"(local object_1 = example.FinalClass(423, false)
local object_2 = example.function_test_class_construct(object_1)
)";

TEST_F(TestSuite, function_test_class_construct_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  FinalClass object1(423, false);
  EXPECT_EQ(object1, function_test_class_construct(object1));

  SCOPED_TRACE("function_test_class_construct_test_script");
  testScript(function_test_class_construct_test_script, session);
}

TEST_F(TestSuite, function_test_class_construct_ptr_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  FinalClass object1(423, false);
  function_test_class_construct(&object1);

  SCOPED_TRACE("function_test_class_construct_test_script");
  testScript(function_test_class_construct_test_script, session);
}

const char* class_is_reachable_after_copy_constuction_script =
    R"(local object_1 = example.FinalClass()
local object_2 = example.FinalClass(object_1)
object_2:someMethod(129)
)";

TEST_F(TestSuite, class_copy_constuction_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::FinalClass object1;
  auto object2 = object1;
  object2.someMethod(129);

  SCOPED_TRACE("class_is_reachable_after_copy_move_constuction_script");
  testScript(class_is_reachable_after_copy_constuction_script, session);
}

const char* class_is_reachable_after_move_constuction_script =
    R"(local object_1 = example.FinalClass()
object_1:someMethod(129)
)";

TEST_F(TestSuite, class_move_constuction_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::FinalClass object1;
  auto object2 = std::move(object1);
  object2.someMethod(129);

  SCOPED_TRACE("class_is_reachable_after_copy_move_constuction_script");
  testScript(class_is_reachable_after_move_constuction_script, session);
}

const char* class_is_reachable_after_copy_assignment_script =
    R"(local object_1 = example.FinalClass()
local object_2 = example.FinalClass()
object_2 = object_1
object_2:someMethod(129)
)";

TEST_F(TestSuite, class_copy_assignment_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::FinalClass object1;
  gunit_hook::FinalClass object2;
  object2 = object1;
  object2.someMethod(129);

  SCOPED_TRACE("class_is_reachable_after_copy_move_assignment_script");
  testScript(class_is_reachable_after_copy_assignment_script, session);
}

const char* class_is_reachable_after_copy_move_assignment_script =
    R"(local object_1 = example.FinalClass()
local object_2 = example.FinalClass()
object_2 = object_1
object_2:someMethod(129)
)";

TEST_F(TestSuite, class_move_assignment_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit_hook::FinalClass object1;
  gunit_hook::FinalClass object2;
  object2 = std::move(object1);
  object2.someMethod(129);

  SCOPED_TRACE("class_is_reachable_after_copy_move_assignment_script");
  testScript(class_is_reachable_after_copy_move_assignment_script, session);
}

const char* function_make_class_construct_obj_test_script =
    R"(local object_1 = example.function_make_class_construct_obj()
object_1:someMethod(2345)
)";

TEST_F(TestSuite, function_make_class_construct_obj_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  auto object = gunit_hook::function_make_class_construct_obj();
  object.someMethod(2345);

  SCOPED_TRACE("function_make_class_construct_obj_test_script");
  testScript(function_make_class_construct_obj_test_script, session);
}

const char* function_make_class_construct_obj_ptr_test_script =
    R"(local object_1 = example.function_make_class_construct_obj_ptr()
object_1:someMethod(2345)
)";

TEST_F(TestSuite, function_make_class_construct_obj_ptr_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  auto object = gunit_hook::function_make_class_construct_obj_ptr();
  object->someMethod(2345);
  delete object;

  SCOPED_TRACE("function_make_class_construct_obj_ptr_test_script");
  testScript(function_make_class_construct_obj_ptr_test_script, session);
}

TEST_F(TestSuite, unreachable_object_error) {
  gunit_hook::FinalClass object;

  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  object.someMethod(23);

  EXPECT_THROW(session->getScript(), ScriptGenerationError);
}
