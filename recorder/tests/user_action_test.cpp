#include <gtest/gtest.h>

#include "recorder/recorder.h"

#include "models/free_functions.h"

using namespace gunit::recorder;
using namespace gunit;

class UserActionTestSuite : public testing::Test {
 public:
};

TEST_F(UserActionTestSuite, calculate_factorial_test) {
  auto session = makeRecordingSession();

  EXPECT_EQ(120, calculate_factorial(5));

  auto script = session->getScript();
  EXPECT_EQ("calculate_factorial(5)\n", script);
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
