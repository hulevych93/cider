#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <lua.hpp>

class ScriptsTestSuite : public testing::Test {};

#ifdef __cplusplus
extern "C" {
#endif

int luaopen_example(lua_State* L);

#ifdef __cplusplus
}
#endif

using LuaStateUPtr = std::unique_ptr<lua_State, void (*)(lua_State*)>;

LuaStateUPtr get_lua() {
  lua_State* _L = luaL_newstate();
  luaL_openlibs(_L);
  luaopen_example(_L);
  return LuaStateUPtr(_L, lua_close);
}

bool executeScript(lua_State* L, const char* script) {
  auto isSuccess = luaL_loadstring(L, script) == LUA_OK &&
                   lua_pcall(L, 0, LUA_MULTRET, 0) == LUA_OK;
  if (!isSuccess) {
    std::cout << "Script execution error: " << lua_tostring(L, -1);
  }
  return isSuccess;
}

TEST_F(ScriptsTestSuite, calculate_factorial) {
  const char* Script = R"(
local result = example.calculate_factorial(5)
assert(result == 120, 'factorial of 5 is 120')
)";

  auto lState = get_lua();
  EXPECT_TRUE(executeScript(lState.get(), Script));
}

TEST_F(ScriptsTestSuite, is_this_sparta_word) {
  const char* Script = R"(
assert(example.is_this_sparta_word('sparta'), 'yes')
assert(not example.is_this_sparta_word('not'), 'no')
)";

  auto lState = get_lua();
  EXPECT_TRUE(executeScript(lState.get(), Script));
}
