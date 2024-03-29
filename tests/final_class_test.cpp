// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "utils/test_suite.h"

#include "final_class.h"

using namespace cider::recorder;
using namespace cider::models;
using namespace cider::tests;
using namespace cider;

struct FinalClassTest : TestSuite {};

TEST_F(FinalClassTest, class_construct_test) {
  {
    auto session = makeLuaRecordingSession(LuaExampleModuleName);

    hook::FinalClass object1;
    EXPECT_EQ("object_1 = example.FinalClass()\n", session->getScript(999U));
  }

  auto session = makeLuaRecordingSession(LuaExampleModuleName);
  hook::FinalClass object2(125, true);
  EXPECT_EQ("object_1 = example.FinalClass(example.Int(125), true)\n",
            session->getScript(999U));
}

const char* class_method_test_script =
    R"(object_1 = example.FinalClass()
object_1:someMethod(example.Int(129))
)";

TEST_F(FinalClassTest, class_method_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::FinalClass object1;
  object1.someMethod(129);

  SCOPED_TRACE("class_method_test_script");
  testScript(class_method_test_script, session);
}

const char* function_test_class_construct_test_script =
    R"(object_1 = example.FinalClass(example.Int(423), false)
object_2 = example.function_test_class_construct(object_1)
object_1:equalEqualOp(object_2)
)";

TEST_F(FinalClassTest, function_test_class_construct_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::FinalClass object1(423, false);
  EXPECT_EQ(object1, hook::function_test_class_construct(object1));

  SCOPED_TRACE("function_test_class_construct_test_script");
  testScript(function_test_class_construct_test_script, session);
}

const char* function_test_class_construct_ptr_test_script =
    R"(object_1 = example.FinalClass(example.Int(423), false)
object_2 = example.function_test_class_construct(object_1)
)";

TEST_F(FinalClassTest, function_test_class_construct_ptr_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::FinalClass object1(423, false);
  hook::function_test_class_construct(&object1);

  SCOPED_TRACE("function_test_class_construct_ptr_test_script");
  testScript(function_test_class_construct_ptr_test_script, session);
}

const char* function_test_class_construct_same_pointer_return_test_script =
    R"(object_1 = example.FinalClass(example.Int(423), false)
object_2 = example.function_test_class_construct_same_pointer_return(object_1)
)";

TEST_F(
    FinalClassTest,
    DISABLED_function_test_class_construct_same_pointer_return_test_script_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::FinalClass object1(423, false);
  hook::function_test_class_construct_same_pointer_return(
      &object1);  // return same object ptr inside new lua object instance What
                  // TO DO?

  SCOPED_TRACE("function_test_class_construct_same_pointer_return_test_script");
  testScript(function_test_class_construct_same_pointer_return_test_script,
             session);
}

const char* class_is_reachable_after_copy_constuction_script =
    R"(object_1 = example.FinalClass()
object_2 = example.FinalClass(object_1)
object_2:someMethod(example.Int(129))
)";

TEST_F(FinalClassTest, class_copy_constuction_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::FinalClass object1;
  auto object2 = object1;
  object2.someMethod(129);

  SCOPED_TRACE("class_is_reachable_after_copy_move_constuction_script");
  testScript(class_is_reachable_after_copy_constuction_script, session);
}

const char* class_is_reachable_after_move_constuction_script =
    R"(object_1 = example.FinalClass()
object_1:someMethod(example.Int(129))
)";

TEST_F(FinalClassTest, class_move_constuction_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::FinalClass object1;
  auto object2 = std::move(object1);
  object2.someMethod(129);

  SCOPED_TRACE("class_is_reachable_after_copy_move_constuction_script");
  testScript(class_is_reachable_after_move_constuction_script, session);
}

const char* class_is_reachable_after_copy_assignment_script =
    R"(object_1 = example.FinalClass()
object_2 = example.FinalClass()
object_2 = object_1
object_2:someMethod(example.Int(129))
)";

TEST_F(FinalClassTest, class_copy_assignment_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::FinalClass object1;
  hook::FinalClass object2;
  object2 = object1;
  object2.someMethod(129);

  SCOPED_TRACE("class_is_reachable_after_copy_move_assignment_script");
  testScript(class_is_reachable_after_copy_assignment_script, session);
}

const char* class_is_reachable_after_copy_move_assignment_script =
    R"(object_1 = example.FinalClass()
object_2 = example.FinalClass()
object_2 = object_1
object_2:someMethod(example.Int(129))
)";

TEST_F(FinalClassTest, class_move_assignment_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::FinalClass object1;
  hook::FinalClass object2;
  object2 = std::move(object1);
  object2.someMethod(129);

  SCOPED_TRACE("class_is_reachable_after_copy_move_assignment_script");
  testScript(class_is_reachable_after_copy_move_assignment_script, session);
}

const char* function_make_class_construct_obj_test_script =
    R"(object_1 = example.function_make_class_construct_obj()
object_1:someMethod(example.Int(2345))
)";

TEST_F(FinalClassTest, function_make_class_construct_obj_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  auto object = hook::function_make_class_construct_obj();
  object.someMethod(2345);

  SCOPED_TRACE("function_make_class_construct_obj_test_script");
  testScript(function_make_class_construct_obj_test_script, session);
}

const char* function_make_class_construct_obj_ptr_test_script =
    R"(object_1 = example.function_make_class_construct_obj_ptr()
object_1:someMethod(example.Int(2345))
)";

TEST_F(FinalClassTest, function_make_class_construct_obj_ptr_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  auto object = hook::function_make_class_construct_obj_ptr();
  object->someMethod(2345);
  delete object;

  SCOPED_TRACE("function_make_class_construct_obj_ptr_test_script");
  testScript(function_make_class_construct_obj_ptr_test_script, session);
}

TEST_F(FinalClassTest, unreachable_object_error) {
  hook::FinalClass object;

  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  object.someMethod(23);

  EXPECT_THROW(session->getScript(999U), ScriptGenerationError);
}
