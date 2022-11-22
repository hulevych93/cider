#include <gtest/gtest.h>

#include "recorder/recorder.h"

#include "models/aggregate_struct.h"
#include "models/enum_test.h"
#include "models/free_functions.h"

using namespace gunit::recorder;
using namespace gunit;

class RecordingTestSuite : public testing::Test {
 public:
};

TEST_F(RecordingTestSuite, script_session_clears_test) {
  auto session = makeLuaRecordingSession();

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
  auto session = makeLuaRecordingSession();

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
  auto session = makeLuaRecordingSession();

  EXPECT_FALSE(models::is_this_sparta_word("something"));
  EXPECT_TRUE(models::is_this_sparta_word("sparta"));

  auto script = session->getScript();
  EXPECT_EQ(is_this_sparta_word_test_script, script);
}

const char* function_test_aggregate_test_script =
    R"(local aggregate = example.Aggregate()
aggregate.condition = true
aggregate.number = 10
example.function_test_aggregate(aggregate)
)";

TEST_F(RecordingTestSuite, function_test_aggregate_test) {
  auto session = makeLuaRecordingSession();

  models::Aggregate aggregateStruct{10, true};
  EXPECT_EQ(aggregateStruct, models::function_test_aggregate(aggregateStruct));

  auto script = session->getScript();
  EXPECT_EQ(function_test_aggregate_test_script, script);
}

const char* function_test_enumeration_test_script =
    R"(example.function_test_enumeration(example.SomeEnumeration_second_value)
)";

TEST_F(RecordingTestSuite, function_test_enumeration_test) {
  auto session = makeLuaRecordingSession();

  models::SomeEnumeration arg = models::SomeEnumeration::second_value;
  EXPECT_EQ(arg, models::function_test_enumeration(arg));

  auto script = session->getScript();
  EXPECT_EQ(function_test_enumeration_test_script, script);
}

const char* summ_these_two_params_test_script =
    R"(example.summ_these_two_params(7, 15)
)";

TEST_F(RecordingTestSuite, summ_these_two_params_test) {
  auto session = makeLuaRecordingSession();

  EXPECT_EQ(22, models::summ_these_two_params(7, 15u));

  auto script = session->getScript();
  EXPECT_EQ(summ_these_two_params_test_script, script);
}
