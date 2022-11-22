#include "interpreter.h"

#include <lua.hpp>

#include <iostream>

namespace gunit {
namespace scripting {

LuaStateUPtr get_lua() {
  lua_State* _L = luaL_newstate();
  luaL_openlibs(_L);
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

}  // namespace scripting
}  // namespace gunit
