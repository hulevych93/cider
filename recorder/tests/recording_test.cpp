#include <gtest/gtest.h>

#include "recorder/recorder.h"

#include "models/aggregate_struct.h"
#include "models/free_functions.h"
#include "models/enum_test.h"

using namespace gunit::recorder;
using namespace gunit;

class UserActionTestSuite : public testing::Test {
 public:
};

const char* calculate_factorial_test_script =
    R"(calculate_factorial(5)
calculate_factorial(6)
)";

TEST_F(UserActionTestSuite, calculate_factorial_test) {
  auto session = makeRecordingSession();

  EXPECT_EQ(120, models::calculate_factorial(5));
  EXPECT_EQ(720, models::calculate_factorial(6));

  auto script = session->getScript();
  EXPECT_EQ(calculate_factorial_test_script, script);
}

const char* is_this_sparta_word_test_script =
    R"(is_this_sparta_word('something')
is_this_sparta_word('sparta')
)";

TEST_F(UserActionTestSuite, is_this_sparta_word_test) {
  auto session = makeRecordingSession();

  EXPECT_FALSE(models::is_this_sparta_word("something"));
  EXPECT_TRUE(models::is_this_sparta_word("sparta"));

  auto script = session->getScript();
  EXPECT_EQ(is_this_sparta_word_test_script, script);
}

const char* function_test_aggregate_test_script =
    R"(local aggr = example.Aggregate()
aggr.condition = true
aggr.number = 10
function_test_aggregate(aggr)
)";

TEST_F(UserActionTestSuite, function_test_aggregate_test) {
  auto session = makeRecordingSession();

  models::Aggregate aggregateStruct{10, true};
  EXPECT_EQ(aggregateStruct, models::function_test_aggregate(aggregateStruct));

  auto script = session->getScript();
  EXPECT_EQ(function_test_aggregate_test_script, script);
}

const char* function_test_enumeration_test_script =
    R"(function_test_enumeration(example.SomeEnumeration_second_value)
)";

TEST_F(UserActionTestSuite, function_test_enumeration_test) {
    auto session = makeRecordingSession();

    models::SomeEnumeration arg = models::SomeEnumeration::second_value;
    EXPECT_EQ(arg, models::function_test_enumeration(arg));

    auto script = session->getScript();
    EXPECT_EQ(function_test_enumeration_test_script, script);
}
