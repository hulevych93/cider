// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "verification-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "pipelines/metrics.h"

#include <iostream>
#include <thread>

namespace cider {
namespace pipelines {

bool ResultsVerificationStage::process(const std::string&,
                                       const std::string& libName,
                                       const cider::Cmd& cmd) {
  std::filesystem::path outPath(cmd.resultsDir);
  std::error_code ec;
  std::filesystem::create_directories(outPath, ec);
  if (ec) {
    std::cerr << "Cannot create dir: " << outPath << " : " << ec.message()
              << std::endl;
    return false;
  }
  outPath /= "verified_results.bin";

  auto results = getResults();
  std::cout << "[VERIFY] Recalculating coverage for methods: " << results.size()
            << std::endl;

  size_t methodIndex = 0;
  for (auto& methodPair : results) {
    ++methodIndex;
    const auto& methodName = methodPair.first;
    auto& methodStats = methodPair.second;

    std::cout << "\n[METHOD " << methodIndex << "/" << results.size() << "] "
              << methodName << " | entries=" << methodStats.entries.size()
              << std::endl;

    cider::gcov_coverage::CoverageMeasurment gcovMeasurer{cmd, libName.c_str()};
    cider::cfg_coverage::CoverageMeasurment cfgMeasurer{cmd, libName.c_str()};

    size_t entryIndex = 0;
    for (auto& entry : methodStats.entries) {
      ++entryIndex;
      std::cout << "  [ENTRY " << entryIndex << "/"
                << methodStats.entries.size()
                << "] TestCase=" << entry.testCaseName
                << " | oldActions=" << entry.oldActions.size()
                << " | newActions=" << entry.newActions.size()
                << " | prevTime=" << entry.timeElapsedMcs << " mcs"
                << std::endl;

      // OLD reports
      const auto oldGcov = gcovMeasurer.getReport(entry.oldActions);
      const auto oldCfg = cfgMeasurer.getReport(entry.oldActions);
      std::this_thread::sleep_for(std::chrono::milliseconds(5));

      // NEW reports
      const auto newGcov = gcovMeasurer.getReport(entry.newActions);
      const auto newCfg = cfgMeasurer.getReport(entry.newActions);

      if (oldGcov.has_value()) {
        entry.oldReport = oldGcov->report;
        std::cout << "    [GCOV-OLD] Coverage recalculated";
      } else {
        std::cout << "    [GCOV-OLD] FAILED" << std::endl;
      }

      if (newGcov.has_value()) {
        entry.newReport = newGcov->report;
        std::cout << "    [GCOV-NEW] Coverage recalculated";
      } else {
        std::cout << "    [GCOV-NEW] FAILED" << std::endl;
      }

      if (oldCfg.has_value()) {
        entry.oldCfgReport = oldCfg.value();
        entry.oldExecutionTimeMcs = oldCfg->meassureTimeMcs;
        std::cout << "    [CFG-OLD] Coverage ok, time="
                  << entry.oldExecutionTimeMcs << " mcs" << std::endl;
      } else {
        std::cout << "    [CFG-OLD] FAILED" << std::endl;
      }

      if (newCfg.has_value()) {
        entry.newCgfReport = newCfg.value();
        entry.newExecutionTimeMcs = newCfg->meassureTimeMcs;
        std::cout << "    [CFG-NEW] Coverage ok, time="
                  << entry.newExecutionTimeMcs << " mcs" << std::endl;
      } else {
        std::cout << "    [CFG-NEW] FAILED" << std::endl;
      }
    }

    std::cout << "[METHOD DONE] " << methodName << std::endl;
  }

  serialization::Serializer serializer;
  serializer << results;
  serializer.save(outPath);
  std::cout << "\n[VERIFY] Updated results saved to: " << outPath << std::endl;

  return true;
}

}  // namespace pipelines
}  // namespace cider
