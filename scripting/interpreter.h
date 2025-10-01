// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <memory>

#include "coverage/cfg_coverage.h"

struct lua_State;

namespace cider {
namespace scripting {

using LuaStateUPtr = std::unique_ptr<lua_State, void (*)(lua_State*)>;

LuaStateUPtr get_lua();

class LuaActionHook {
 public:
  using HookInitFn = std::function<void(int)>;
  using HookFn = std::function<void(lua_State*, int)>;

  static LuaActionHook& instance() {
    static LuaActionHook inst;
    return inst;
  }

  void setHook(lua_State* L, HookInitFn initFn, HookFn fn);

  void clear(lua_State* L);

 private:
  LuaActionHook() = default;
  ~LuaActionHook() = default;

  static int cider_mark(lua_State* L);
  static int l_cider_mark_init(lua_State* L);

  HookInitFn initFunc_;
  HookFn functor_;
};

bool executeScript(lua_State* L, const char* script);

class CoverageCollector final {
 public:
  CoverageCollector(lua_State* L, bool collectFineTracks = false);
  ~CoverageCollector();

  cider::cfg_coverage::CoveragePerAction& getTrace() { return traceCoverage_; }
  const cider::cfg_coverage::CoveragePerAction& getTrace() const {
    return traceCoverage_;
  }

 private:
  void onAction(lua_State* L, int idx);

 private:
  lua_State* lState;
  std::unique_ptr<int> blockCount_;
  std::chrono::steady_clock::time_point startTime_;
  bool collectFineTracks_;

  cider::cfg_coverage::CoveragePerAction traceCoverage_;
  std::vector<std::uint8_t> seenBlocks_;
};

}  // namespace scripting
}  // namespace cider
