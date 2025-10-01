// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

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
    std::cout << popResult(L);
  }
  return ok;
}

int LuaActionHook::cider_mark(lua_State* L) {
  int idx = luaL_checkinteger(L, 1);
  LuaActionHook& self = instance();
  if (self.functor_) {
    self.functor_(L, idx);
  }
  return 0;
}

int LuaActionHook::l_cider_mark_init(lua_State* L) {
  int total = luaL_checkinteger(L, 1);
  LuaActionHook& self = instance();
  if (self.initFunc_) {
    self.initFunc_(total);
  }
  return 0;
}

void LuaActionHook::setHook(lua_State* L, HookInitFn initFn, HookFn fn) {
  functor_ = std::move(fn);
  initFunc_ = std::move(initFn);

  lua_pushcfunction(L, l_cider_mark_init);
  lua_setglobal(L, "cider_mark_init");

  lua_pushcfunction(L, cider_mark);
  lua_setglobal(L, "cider_mark");
}

void LuaActionHook::clear(lua_State* L) {
  lua_pushnil(L);
  lua_setglobal(L, "cider_mark");
  functor_ = nullptr;

  lua_pushnil(L);
  lua_setglobal(L, "cider_mark_init");
  initFunc_ = nullptr;
}

CoverageCollector::CoverageCollector(lua_State* L, bool collectFineTracks)
    : lState(L), collectFineTracks_(collectFineTracks) {
  blockCount_ = std::make_unique<int>(0);
  startTime_ = std::chrono::steady_clock::now();

  cider::scripting::LuaActionHook::instance().setHook(
      lState,
      [&](int numActions) {
        traceCoverage_.resize(numActions);
        seenBlocks_.clear();
        if (*blockCount_ > 0) {
          seenBlocks_.resize(*blockCount_, 0);
        }
      },
      [&](lua_State* lua, int idx) { onAction(lua, idx); });

  cider::cfg_coverage::zeroCfgCounters(blockCount_.get());
}

CoverageCollector::~CoverageCollector() {
  cider::scripting::LuaActionHook::instance().clear(lState);
}

void CoverageCollector::onAction(lua_State* L, int idx) {
  (void)L;

  if (seenBlocks_.empty() && *blockCount_ > 0) {
    seenBlocks_.resize(*blockCount_, 0);
  }

  auto cov = cider::cfg_coverage::getCoverage();
  cov.meassureTimeMcs = std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::steady_clock::now() - startTime_)
                            .count();
  cov.status = true;

  bool unique = false;
  std::vector<std::uint8_t> delta(cov.coveredTracks.size(), 0);
  for (size_t j = 0; j < cov.coveredTracks.size(); ++j) {
    if (cov.coveredTracks[j] && !seenBlocks_[j]) {
      delta[j] = 1;
      seenBlocks_[j] = 1;
      unique = true;
    }
  }

  traceCoverage_[idx].hasUnique = unique;
  if (collectFineTracks_) {
    traceCoverage_[idx].coveredTracks = std::move(delta);
  }
  traceCoverage_[idx].meassureTimeMcs = cov.meassureTimeMcs;
  traceCoverage_[idx].status = cov.status;
}

}  // namespace scripting
}  // namespace cider
