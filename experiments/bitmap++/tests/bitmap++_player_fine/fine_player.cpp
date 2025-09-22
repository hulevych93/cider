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
    std::string script;
    if (argc == 2) {
      script = cider::loadFile(argv[1]);
    } else {
      for (std::string line; std::getline(std::cin, line);) {
        script += line += "\n";
      }
    }

    auto lState = cider::scripting::get_lua();
    luaopen_bitmap_cplusplus(lState.get());

    cider::cfg_coverage::CoveragePerAction traceCoverage;

    const auto startTime = std::chrono::steady_clock::now();

    int* blockCount = new int;
    *blockCount = 0;

    cider::scripting::LuaActionHook::instance().setHook(
        lState.get(), [&](int numActions) { traceCoverage.resize(numActions); },
        [&](lua_State* L, int idx) {
          (void)L;  // unused

          static std::vector<std::uint8_t> seenBlocks;
          if (seenBlocks.empty()) {
            assert(blockCount);
            seenBlocks.resize(*blockCount, 0);
          }

          cider::cfg_coverage::Coverage cov =
              cider::cfg_coverage::getCoverage();

          const auto end = std::chrono::steady_clock::now();
          cov.meassureTimeMcs =
              std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                    startTime)
                  .count();

          cov.status = true;

          std::vector<std::uint8_t> delta(cov.coveredTracks.size(), 0);
          for (size_t j = 0; j < cov.coveredTracks.size(); ++j) {
            if (cov.coveredTracks[j] && !seenBlocks[j]) {
              delta[j] = 1;
              seenBlocks[j] = 1;
            }
          }

          traceCoverage[idx].coveredTracks = std::move(delta);
          traceCoverage[idx].meassureTimeMcs = cov.meassureTimeMcs;
          traceCoverage[idx].status = cov.status;
        });

    cider::cfg_coverage::zeroCfgCounters(blockCount);

    const auto result =
        cider::scripting::executeScript(lState.get(), script.c_str()) ? 0 : 1;

    cider::scripting::LuaActionHook::instance().clear(lState.get());

    cider::cfg_coverage::dumpCoverageToCout(traceCoverage);

    return result;
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}
