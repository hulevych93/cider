#include "utils/test_suite.h"

#include "gunit-models/free_functions.h"

using namespace gunit::recorder;
using namespace gunit::models;
using namespace gunit::tests;
using namespace gunit;

struct FreeFunctionsTest : TestSuite {};

const char* calculate_factorial_test_script =
    R"(example.calculate_factorial(5)
example.calculate_factorial(6)
)";

TEST_F(FreeFunctionsTest, calculate_factorial_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  EXPECT_EQ(120, gunit_hook::calculate_factorial(5));
  EXPECT_EQ(720, gunit_hook::calculate_factorial(6));

  SCOPED_TRACE("calculate_factorial_test_script");
  testScript(calculate_factorial_test_script, session);
}

const char* is_this_sparta_word_test_script =
    R"(example.is_this_sparta_word('something')
example.is_this_sparta_word('sparta')
)";

TEST_F(FreeFunctionsTest, is_this_sparta_word_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  EXPECT_FALSE(gunit_hook::is_this_sparta_word("something"));
  EXPECT_TRUE(gunit_hook::is_this_sparta_word("sparta"));

  SCOPED_TRACE("is_this_sparta_word_test_script");
  testScript(is_this_sparta_word_test_script, session);
}

const char* summ_these_two_params_test_script =
    R"(example.summ_these_two_params(7, 15)
)";

TEST_F(FreeFunctionsTest, summ_these_two_params_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  EXPECT_EQ(22, gunit_hook::summ_these_two_params(7, 15u));

  SCOPED_TRACE("summ_these_two_params_test_script");
  testScript(summ_these_two_params_test_script, session);
}

TEST_F(FreeFunctionsTest, script_session_clears_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  EXPECT_EQ(120, gunit_hook::calculate_factorial(5));
  EXPECT_EQ("example.calculate_factorial(5)\n", session->getScript());

  EXPECT_EQ(720, gunit_hook::calculate_factorial(6));
  EXPECT_EQ("example.calculate_factorial(6)\n", session->getScript());
}

TEST_F(FreeFunctionsTest, bad_num_cast_script_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);
  (void)session;

  EXPECT_THROW(gunit_hook::summ_these_two_params(
                   0, std::numeric_limits<unsigned int>::max()),
               BadNumCast);
}
