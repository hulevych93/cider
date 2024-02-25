// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include <chrono>
#include <fstream>
#include <iostream>

#include "coverage/coverage.h"
#include "scripting/interpreter.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

int luaopen_pugixml(lua_State* L);

#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[]) {
  try {
    std::string script;
    if (argc == 2) {
      std::cout << "running script: " << argv[1] << std::endl;
      script = cider::coverage::loadFile(argv[1]);
    } else {
      for (std::string line; std::getline(std::cin, line);) {
        script += line += "\n";
      }
    }

    auto lState = cider::scripting::get_lua();
    luaopen_pugixml(lState.get());

    const auto start = std::chrono::system_clock::now();
    const auto result =
        cider::scripting::executeScript(lState.get(), script.c_str()) ? 0 : 1;

    const auto end = std::chrono::system_clock::now();
    const auto elapsed = end - start;
    std::cout << "Time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(elapsed)
                     .count()
              << " mcs" << std::endl;

    return result;
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}
