#include "interpreter.h"

#include <lua.hpp>

namespace gunit {
namespace scripting {

LuaStateUPtr get_lua() {
  lua_State* _L = luaL_newstate();
  luaL_openlibs(_L);
  return LuaStateUPtr(_L, lua_close);
}

bool executeScript(lua_State* L, const char* script) {
  return luaL_loadstring(L, script) == LUA_OK &&
         lua_pcall(L, 0, LUA_MULTRET, 0) == LUA_OK;
}

}  // namespace scripting
}  // namespace gunit
