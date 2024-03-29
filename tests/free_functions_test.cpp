// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "utils/test_suite.h"

#include "free_functions.h"

using namespace cider::recorder;
using namespace cider::models;
using namespace cider::tests;
using namespace cider;

struct FreeFunctionsTest : TestSuite {};

const char* calculate_factorial_test_script =
    R"(example.calculate_factorial(example.Int(5))
example.calculate_factorial(example.Int(6))
)";

TEST_F(FreeFunctionsTest, calculate_factorial_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  EXPECT_EQ(120, hook::calculate_factorial(5));
  EXPECT_EQ(720, hook::calculate_factorial(6));

  SCOPED_TRACE("calculate_factorial_test_script");
  testScript(calculate_factorial_test_script, session);
}

const char* is_this_sparta_word_test_script =
    R"(example.is_this_sparta_word('something')
example.is_this_sparta_word('sparta')
)";

TEST_F(FreeFunctionsTest, is_this_sparta_word_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  EXPECT_FALSE(hook::is_this_sparta_word("something"));
  EXPECT_TRUE(hook::is_this_sparta_word("sparta"));

  SCOPED_TRACE("is_this_sparta_word_test_script");
  testScript(is_this_sparta_word_test_script, session);
}

const char* summ_these_two_params_test_script =
    R"(example.summ_these_two_params(example.Int(7), example.ULong(15))
)";

TEST_F(FreeFunctionsTest, summ_these_two_params_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  EXPECT_EQ(22, hook::summ_these_two_params(7, 15u));

  SCOPED_TRACE("summ_these_two_params_test_script");
  testScript(summ_these_two_params_test_script, session);
}
