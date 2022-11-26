#include <gtest/gtest.h>

#include "recorder/recorder.h"

#include "scripting/interpreter.h"

#include "models/aggregate_struct.h"
#include "models/class_construct.h"
#include "models/enum_test.h"
#include "models/free_functions.h"

#ifdef __cplusplus
extern "C" {
#endif

int luaopen_example(lua_State* L);

#ifdef __cplusplus
}
#endif

using namespace gunit::recorder;
using namespace gunit::scripting;
using namespace gunit;

class RecordingTestSuite : public testing::Test {
 public:
  static std::string testScript(const std::string& script) {
    auto lState = get_lua();
    luaopen_example(lState.get());
    EXPECT_TRUE(executeScript(lState.get(), script.c_str()));
    return script;
  }
};

TEST_F(RecordingTestSuite, script_session_clears_test) {
  auto session = makeLuaRecordingSession("example");

  EXPECT_EQ(120, models::calculate_factorial(5));
  EXPECT_EQ("example.calculate_factorial(5)\n", session->getScript());

  EXPECT_EQ(720, models::calculate_factorial(6));
  EXPECT_EQ("example.calculate_factorial(6)\n",
            testScript(session->getScript()));
}

const char* calculate_factorial_test_script =
    R"(example.calculate_factorial(5)
example.calculate_factorial(6)
)";

TEST_F(RecordingTestSuite, calculate_factorial_test) {
  auto session = makeLuaRecordingSession("example");

  EXPECT_EQ(120, models::calculate_factorial(5));
  EXPECT_EQ(720, models::calculate_factorial(6));

  auto script = session->getScript();
  EXPECT_EQ(calculate_factorial_test_script, testScript(script));
}

const char* is_this_sparta_word_test_script =
    R"(example.is_this_sparta_word('something')
example.is_this_sparta_word('sparta')
)";

TEST_F(RecordingTestSuite, is_this_sparta_word_test) {
  auto session = makeLuaRecordingSession("example");

  EXPECT_FALSE(models::is_this_sparta_word("something"));
  EXPECT_TRUE(models::is_this_sparta_word("sparta"));

  auto script = session->getScript();
  EXPECT_EQ(is_this_sparta_word_test_script, testScript(script));
}

const char* function_test_aggregate_test_script =
    R"(local object_1 = example.Aggregate()
object_1.condition = true
object_1.number = 10
example.function_test_aggregate(object_1)
)";

TEST_F(RecordingTestSuite, function_test_aggregate_test) {
  auto session = makeLuaRecordingSession("example");

  models::Aggregate aggregateStruct{10, true};
  EXPECT_EQ(aggregateStruct, models::function_test_aggregate(aggregateStruct));

  auto script = session->getScript();
  EXPECT_EQ(function_test_aggregate_test_script, testScript(script));
}

const char* function_test_aggregate_ptr_test_script =
    R"(local object_1 = example.Aggregate()
object_1.condition = true
object_1.number = 10
example.function_test_aggregate_ptr(object_1)
)";

TEST_F(RecordingTestSuite, function_test_aggregate_ptr_test) {
  auto session = makeLuaRecordingSession("example");

  models::Aggregate aggregateStruct{10, true};
  EXPECT_EQ(&aggregateStruct,
            models::function_test_aggregate_ptr(&aggregateStruct));

  auto script = session->getScript();
  EXPECT_EQ(function_test_aggregate_ptr_test_script, testScript(script));
}

const char* function_test_enumeration_test_script =
    R"(example.function_test_enumeration(example.SomeEnumeration_second_value)
)";

TEST_F(RecordingTestSuite, function_test_enumeration_test) {
  auto session = makeLuaRecordingSession("example");

  models::SomeEnumeration arg = models::SomeEnumeration::second_value;
  EXPECT_EQ(arg, models::function_test_enumeration(arg));

  auto script = session->getScript();
  EXPECT_EQ(function_test_enumeration_test_script, testScript(script));
}

const char* summ_these_two_params_test_script =
    R"(example.summ_these_two_params(7, 15)
)";

TEST_F(RecordingTestSuite, summ_these_two_params_test) {
  auto session = makeLuaRecordingSession("example");

  EXPECT_EQ(22, models::summ_these_two_params(7, 15u));

  auto script = session->getScript();
  EXPECT_EQ(summ_these_two_params_test_script, testScript(script));
}

TEST_F(RecordingTestSuite, class_construct_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1;
  EXPECT_EQ("local object_1 = example.ClassConstruct()\n",
            session->getScript());

  models::ClassConstruct object2(125, true);
  EXPECT_EQ("local object_1 = example.ClassConstruct(125, true)\n",
            session->getScript());
}

const char* class_method_test_script =
    R"(local object_1 = example.ClassConstruct()
object_1:someMethod(129)
)";

TEST_F(RecordingTestSuite, class_method_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1;
  object1.someMethod(129);

  EXPECT_EQ(class_method_test_script, testScript(session->getScript()));
}

const char* function_test_class_construct_test_script =
    R"(local object_1 = example.ClassConstruct(423, false)
local object_2 = example.function_test_class_construct(object_1)
)";

TEST_F(RecordingTestSuite, function_test_class_construct_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1(423, false);
  EXPECT_EQ(object1, function_test_class_construct(object1));

  EXPECT_EQ(function_test_class_construct_test_script,
            testScript(session->getScript()));
}

const char* function_test_class_construct_ptr_test_script =
    R"(local object_1 = example.ClassConstruct(423, false)
local object_2 = example.function_test_class_construct_ptr(object_1)
)";

TEST_F(RecordingTestSuite, function_test_class_construct_ptr_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1(423, false);
  function_test_class_construct_ptr(&object1);

  EXPECT_EQ(function_test_class_construct_ptr_test_script,
            testScript(session->getScript()));
}

const char* class_is_reachable_after_copy_move_constuction_script =
    R"(local object_1 = example.ClassConstruct()
local object_2 = example.ClassConstruct(object_1)
object_2:someMethod(129)
)";

TEST_F(RecordingTestSuite, class_copy_constuction_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1;
  auto object2 = object1;
  object2.someMethod(129);

  EXPECT_EQ(class_is_reachable_after_copy_move_constuction_script,
            testScript(session->getScript()));
}

TEST_F(RecordingTestSuite, class_move_constuction_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1;
  auto object2 = std::move(object1);
  object2.someMethod(129);

  EXPECT_EQ(class_is_reachable_after_copy_move_constuction_script,
            testScript(session->getScript()));
}

const char* class_is_reachable_after_copy_move_assignment_script =
    R"(local object_1 = example.ClassConstruct()
local object_2 = example.ClassConstruct()
object_2 = object_1
object_2:someMethod(129)
)";

TEST_F(RecordingTestSuite, class_copy_assignment_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1;
  models::ClassConstruct object2;
  object2 = object1;
  object2.someMethod(129);

  EXPECT_EQ(class_is_reachable_after_copy_move_assignment_script,
            testScript(session->getScript()));
}

TEST_F(RecordingTestSuite, class_move_assignment_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1;
  models::ClassConstruct object2;
  object2 = std::move(object1);
  object2.someMethod(129);

  EXPECT_EQ(class_is_reachable_after_copy_move_assignment_script,
            testScript(session->getScript()));
}

const char* function_make_class_construct_obj_test_script =
    R"(local object_1 = example.function_make_class_construct_obj()
object_1:someMethod(2345)
)";

TEST_F(RecordingTestSuite, function_make_class_construct_obj_test) {
  auto session = makeLuaRecordingSession("example");

  auto object = models::function_make_class_construct_obj();
  object.someMethod(2345);

  EXPECT_EQ(function_make_class_construct_obj_test_script,
            testScript(session->getScript()));
}

const char* function_make_class_construct_obj_ptr_test_script =
    R"(local object_1 = example.function_make_class_construct_obj_ptr()
object_1:someMethod(2345)
)";

TEST_F(RecordingTestSuite, function_make_class_construct_obj_ptr_test) {
  auto session = makeLuaRecordingSession("example");

  auto object = models::function_make_class_construct_obj_ptr();
  object->someMethod(2345);
  delete object;

  EXPECT_EQ(function_make_class_construct_obj_ptr_test_script,
            testScript(session->getScript()));
}
