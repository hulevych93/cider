// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

struct lua_State;

namespace cider {
namespace scripting {

using LuaStateUPtr = std::unique_ptr<lua_State, void (*)(lua_State*)>;

LuaStateUPtr get_lua();

bool executeScript(lua_State* L, const char* script);

}  // namespace scripting
}  // namespace cider
