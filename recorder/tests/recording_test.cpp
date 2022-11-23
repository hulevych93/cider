#include <gtest/gtest.h>

#include "recorder/recorder.h"

#include "models/aggregate_struct.h"
#include "models/class_construct.h"
#include "models/enum_test.h"
#include "models/free_functions.h"

using namespace gunit::recorder;
using namespace gunit;

class RecordingTestSuite : public testing::Test {
 public:
};

TEST_F(RecordingTestSuite, script_session_clears_test) {
  auto session = makeLuaRecordingSession("example");

  EXPECT_EQ(120, models::calculate_factorial(5));
  EXPECT_EQ("example.calculate_factorial(5)\n", session->getScript());

  EXPECT_EQ(720, models::calculate_factorial(6));
  EXPECT_EQ("example.calculate_factorial(6)\n", session->getScript());
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
  EXPECT_EQ(calculate_factorial_test_script, script);
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
  EXPECT_EQ(is_this_sparta_word_test_script, script);
}

const char* function_test_aggregate_test_script =
    R"(local aggregate1 = example.Aggregate()
aggregate1.condition = true
aggregate1.number = 10
example.function_test_aggregate(aggregate1)
)";

TEST_F(RecordingTestSuite, function_test_aggregate_test) {
  auto session = makeLuaRecordingSession("example");

  models::Aggregate aggregateStruct{10, true};
  EXPECT_EQ(aggregateStruct, models::function_test_aggregate(aggregateStruct));

  auto script = session->getScript();
  EXPECT_EQ(function_test_aggregate_test_script, script);
}

const char* function_test_enumeration_test_script =
    R"(example.function_test_enumeration(example.SomeEnumeration_second_value)
)";

TEST_F(RecordingTestSuite, function_test_enumeration_test) {
  auto session = makeLuaRecordingSession("example");

  models::SomeEnumeration arg = models::SomeEnumeration::second_value;
  EXPECT_EQ(arg, models::function_test_enumeration(arg));

  auto script = session->getScript();
  EXPECT_EQ(function_test_enumeration_test_script, script);
}

const char* summ_these_two_params_test_script =
    R"(example.summ_these_two_params(7, 15)
)";

TEST_F(RecordingTestSuite, summ_these_two_params_test) {
  auto session = makeLuaRecordingSession("example");

  EXPECT_EQ(22, models::summ_these_two_params(7, 15u));

  auto script = session->getScript();
  EXPECT_EQ(summ_these_two_params_test_script, script);
}

TEST_F(RecordingTestSuite, class_construct_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1;
  EXPECT_EQ("local ClassConstruct1 = example.ClassConstruct()\n",
            session->getScript());

  models::ClassConstruct object2(125, true);
  EXPECT_EQ("local ClassConstruct1 = example.ClassConstruct(125, true)\n",
            session->getScript());
}

const char* class_method_test_script =
    R"(local ClassConstruct1 = example.ClassConstruct()
ClassConstruct1:someMethod(129)
)";

TEST_F(RecordingTestSuite, class_method_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1;
  object1.someMethod(129);

  EXPECT_EQ(class_method_test_script, session->getScript());
}

const char* constructed_class_as_param_test_script =
    R"(local ClassConstruct1 = example.ClassConstruct(423, false)
example.function_test_class_construct(ClassConstruct1)
)";

TEST_F(RecordingTestSuite, constructed_class_as_param_test) {
  auto session = makeLuaRecordingSession("example");

  models::ClassConstruct object1(423, false);
  function_test_class_construct(object1);

  EXPECT_EQ(constructed_class_as_param_test_script, session->getScript());
}
