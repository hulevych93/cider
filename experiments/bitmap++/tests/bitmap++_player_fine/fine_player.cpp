// Copyright (C) 2022-2025 Hulevych Mykhailo
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

int luaopen_bitmap_cplusplus(lua_State* L);

#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[]) {
  try {
    auto lState = cider::scripting::get_lua();
    luaopen_bitmap_cplusplus(lState.get());

    if (argc == 2) {
      const auto script = cider::loadFile(argv[1]);

      cider::scripting::CoverageCollector coverageCollector(lState.get(),
                                                            false);
      int result =
          cider::scripting::executeScript(lState.get(), script.c_str()) ? 0 : 1;
      cider::cfg_coverage::dumpCoverageToCout(coverageCollector.getTrace());
      return result;
    }

    std::string line;
    while (std::getline(std::cin, line)) {
      if (line == "QUIT")
        break;

      if (line == "RUN") {
        std::string script, line;
        while (std::getline(std::cin, line)) {
          if (line == "<<<END>>>")
            break;
          script += line + "\n";
        }

        cider::scripting::CoverageCollector coverageCollector(lState.get(),
                                                              false);

        bool ok = cider::scripting::executeScript(lState.get(), script.c_str());

        cider::cfg_coverage::dumpCoverageToCout(coverageCollector.getTrace());

        std::cout << (ok ? "OK" : "FAIL") << "\nEND\n" << std::flush;
      }
    }
    return 0;

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }
}
