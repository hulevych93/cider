#include <gtest/gtest.h>

#include "recorder/recorder.h"

#include "models/aggregate_struct.h"
#include "models/free_functions.h"

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

  EXPECT_EQ(120, calculate_factorial(5));
  EXPECT_EQ(720, calculate_factorial(6));

  auto script = session->getScript();
  EXPECT_EQ(calculate_factorial_test_script, script);
}

const char* is_this_sparta_word_test_script =
    R"(is_this_sparta_word('something')
is_this_sparta_word('sparta')
)";

TEST_F(UserActionTestSuite, is_this_sparta_word_test) {
  auto session = makeRecordingSession();

  EXPECT_FALSE(is_this_sparta_word("something"));
  EXPECT_TRUE(is_this_sparta_word("sparta"));

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

  Aggregate aggregateStruct{10, true};
  EXPECT_EQ(aggregateStruct, function_test_aggregate(aggregateStruct));

  auto script = session->getScript();
  EXPECT_EQ(function_test_aggregate_test_script, script);
}
