// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "verification-pipe.h"

#include "recorder/details/generator.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "pipelines/metrics.h"

#include <tlog.h>
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
  tlog_info << "[VERIFY] Recalculating coverage for methods: " << results.size()
            << std::endl;

  size_t methodIndex = 0;
  for (auto& methodPair : results) {
    ++methodIndex;
    const auto& methodName = methodPair.first;
    auto& methodStats = methodPair.second;

    tlog_info << "\n[METHOD " << methodIndex << "/" << results.size() << "] "
              << methodName << " | entries=" << methodStats.entries.size()
              << std::endl;

    cider::gcov_coverage::CoverageMeasurment gcovMeasurer{cmd, libName.c_str()};
    cider::cfg_coverage::CoverageMeasurment cfgMeasurer{cmd, libName.c_str()};

    size_t entryIndex = 0;
    for (auto& entry : methodStats.entries) {
      ++entryIndex;
      tlog_info << "  [ENTRY " << entryIndex << "/"
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
        tlog_info << "    [GCOV-OLD] Coverage recalculated";
      } else {
        tlog_info << "    [GCOV-OLD] FAILED" << std::endl;
      }

      if (newGcov.has_value()) {
        entry.newReport = newGcov->report;
        tlog_info << "    [GCOV-NEW] Coverage recalculated";
      } else {
        tlog_info << "    [GCOV-NEW] FAILED" << std::endl;
      }

      if (oldCfg.has_value()) {
        entry.oldCfgReport = oldCfg.value();
        entry.oldExecutionTimeMcs = oldCfg->meassureTimeMcs;
        tlog_info << "    [CFG-OLD] Coverage ok, time="
                  << entry.oldExecutionTimeMcs << " mcs" << std::endl;
      } else {
        tlog_info << "    [CFG-OLD] FAILED" << std::endl;
      }

      if (newCfg.has_value()) {
        entry.newCgfReport = newCfg.value();
        entry.newExecutionTimeMcs = newCfg->meassureTimeMcs;
        tlog_info << "    [CFG-NEW] Coverage ok, time="
                  << entry.newExecutionTimeMcs << " mcs" << std::endl;
      } else {
        tlog_info << "    [CFG-NEW] FAILED" << std::endl;
      }
    }

    tlog_info << "[METHOD DONE] " << methodName << std::endl;
  }

  serialization::Serializer serializer;
  serializer << results;
  serializer.save(outPath);
  tlog_info << "\n[VERIFY] Updated results saved to: " << outPath << std::endl;

  return true;
}

}  // namespace pipelines
}  // namespace cider
