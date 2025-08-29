// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include <fstream>
#include <iostream>

#include "coverage/cfg_measurer.h"
#include "coverage/coverage.h"
#include "scripting/interpreter.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

int luaopen_hjson(lua_State* L);

#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[]) {
  try {
    std::string script;
    if (argc == 2) {
      script = cider::loadFile(argv[1]);
    } else {
      for (std::string line; std::getline(std::cin, line);) {
        script += line += "\n";
      }
    }

    auto lState = cider::scripting::get_lua();
    luaopen_hjson(lState.get());

    const auto startTime = std::chrono::steady_clock::now();
    auto startCov = cider::cfg_coverage::getCoverage();
    const auto result =
        cider::scripting::executeScript(lState.get(), script.c_str()) ? 0 : 1;

    cider::cfg_coverage::dumpCoverageToCout(result == 0, startCov, startTime);

    return result;
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}
