#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <lua.hpp>

#include "scripting/interpreter.h"

using namespace gunit::scripting;

class ScriptsTestSuite : public testing::Test {};

#ifdef __cplusplus
extern "C" {
#endif

int luaopen_example(lua_State* L);

#ifdef __cplusplus
}
#endif

TEST_F(ScriptsTestSuite, calculate_factorial) {
  const char* Script = R"(
local result = example.calculate_factorial(5)
assert(result == 120, 'factorial of 5 is 120')
)";

  auto lState = get_lua();
  luaopen_example(lState.get());
  EXPECT_TRUE(executeScript(lState.get(), Script));
}

TEST_F(ScriptsTestSuite, is_this_sparta_word) {
  const char* Script = R"(
assert(example.is_this_sparta_word('sparta'), 'yes')
assert(not example.is_this_sparta_word('not'), 'no')
)";

  auto lState = get_lua();
  luaopen_example(lState.get());
  EXPECT_TRUE(executeScript(lState.get(), Script));
}

TEST_F(ScriptsTestSuite, function_test_aggregate) {
  const char* Script = R"(
local result = example.Aggregate()
assert(result == example.function_test_aggregate(result), 'aggregate')
)";

  auto lState = get_lua();
  luaopen_example(lState.get());
  EXPECT_TRUE(executeScript(lState.get(), Script));
}

TEST_F(ScriptsTestSuite, function_test_aggregate_ptr) {
  const char* Script = R"(
local result = example.Aggregate()
assert(result == example.function_test_aggregate_ptr(result), 'aggregate ptr')
)";

  auto lState = get_lua();
  luaopen_example(lState.get());
  EXPECT_TRUE(executeScript(lState.get(), Script));
}
