#pragma once

#include <memory>

struct lua_State;

namespace gunit {
namespace scripting {

using LuaStateUPtr = std::unique_ptr<lua_State, void (*)(lua_State*)>;

LuaStateUPtr get_lua();

bool executeScript(lua_State* L, const char* script);

}  // namespace scripting
}  // namespace gunit
