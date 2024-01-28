#include "interpreter.h"

#include <lua.hpp>

#include <iostream>

namespace cider {
namespace scripting {

namespace {
std::string popResult(lua_State* L) {
  std::string result;

  if (lua_gettop(L) > 0 && lua_isstring(L, -1)) {
    result = std::string{lua_tostring(L, -1)};
    lua_pop(L, 1);
  }

  return result;
}

}  // namespace

LuaStateUPtr get_lua() {
  lua_State* _L = luaL_newstate();
  luaL_openlibs(_L);
  return LuaStateUPtr(_L, lua_close);
}

bool executeScript(lua_State* L, const char* script) {
  const bool ok = luaL_loadstring(L, script) == LUA_OK &&
                  lua_pcall(L, 0, LUA_MULTRET, 0) == LUA_OK;
  if (!ok) {
    std::cerr << popResult(L);
  }
  return ok;
}

}  // namespace scripting
}  // namespace cider
