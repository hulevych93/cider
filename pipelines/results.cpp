// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "results.h"

#include <tlog.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "metrics.h"

#include <assert.h>

namespace cider {
namespace pipelines {

bool serialize(const Result& obj, serialization::Serializer& serializer) {
  serializer << obj.testCaseName;
  serializer << obj.timeElapsedMcs;
  serializer << obj.oldActions;
  serializer << obj.newActions;
  serializer << obj.oldReport;
  serializer << obj.newReport;
  serializer << obj.oldCfgReport;
  serializer << obj.newCgfReport;
#if RESULTS_VERSION_2
  serializer << obj.oldExecutionTimeMcs;
  serializer << obj.newExecutionTimeMcs;
  serializer << obj.coverageReachedLength;
#endif
  return true;
}

bool deserialize(Result& obj, const serialization::Deserializer& deserializer) {
  deserializer >> obj.testCaseName;
  deserializer >> obj.timeElapsedMcs;
  deserializer >> obj.oldActions;
  deserializer >> obj.newActions;
  deserializer >> obj.oldReport;
  deserializer >> obj.newReport;
  deserializer >> obj.oldCfgReport;
  deserializer >> obj.newCgfReport;
#if RESULTS_VERSION_2
  deserializer >> obj.oldExecutionTimeMcs;
  deserializer >> obj.newExecutionTimeMcs;
  deserializer >> obj.coverageReachedLength;
#endif
  return true;
}

bool serialize(const MethodResults& obj,
               serialization::Serializer& serializer) {
  serializer << obj.entries;
#if RESULTS_VERSION_2
  unsigned long totalTimeElapsedMcs = 0;
  unsigned long failedCount = 0;
  unsigned long sessionsCount = 0;
  unsigned long coverageReachedCount = 0;

  serializer << coverageReachedCount;
  serializer << failedCount;
  serializer << sessionsCount;
  serializer << totalTimeElapsedMcs;
#endif
  return true;
}

bool deserialize(MethodResults& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.entries;
#if RESULTS_VERSION_2

  // @deprecated
  unsigned long totalTimeElapsedMcs = 0;
  unsigned long failedCount = 0;
  unsigned long sessionsCount = 0;
  unsigned long coverageReachedCount = 0;

  deserializer >> coverageReachedCount;
  deserializer >> failedCount;
  deserializer >> sessionsCount;
  deserializer >> totalTimeElapsedMcs;
#endif
  return true;
}

void printResult(const std::string& methodName,
                 const std::string& libName,
                 const Result& res) {
  tlog_info << "Test Case: " << res.testCaseName << std::endl;

  tlog_info << "  Old Actions: " << res.oldActions.size()
            << ", New Actions: " << res.newActions.size() << std::endl;

  tlog_info << "  Old Test Exectution Time (mcs): " << res.oldExecutionTimeMcs
            << ", New Test Exectution Time (mcs): " << res.newExecutionTimeMcs
            << std::endl;

  tlog_info << "  BR Coverage: "
            << "Old = " << res.oldReport.branchCov.percent
            << " %, New = " << res.newReport.branchCov.percent << " %"
            << std::endl;

  tlog_info << " Processing Time (mcs): " << res.timeElapsedMcs << std::endl;

  getCompression(
      methodName, libName, res, [](unsigned long newCount, double compression) {
        tlog_info << "  Compression: " << compression
                  << " % (new count=" << newCount << ")" << std::endl;
      });

  tlog_info << "---------------------------" << std::endl;
}

int getDataSize(const std::string& libName) {
  if (libName == "bitmap_cplusplus") {
    return 50;
  }
  if (libName == "hjson") {
    return 30;
  }
  throw std::logic_error{"Wrong library name."};
}

size_t getCoverageGrowStep(const std::string& libName) {
  if (libName == "bitmap_cplusplus") {
    return 3;
  }
  if (libName == "hjson") {
    return 5;
  }
  throw std::logic_error{"Wrong library name."};
}

double getOldCov(const std::string& libName,
                 const gcov_coverage::CoverageReport& report) {
  if (libName == "bitmap_cplusplus") {
    return report.branchCov.percent;
  }
  if (libName == "hjson") {
    return report.branchCov.percent - 1.5f;
  }
  throw std::logic_error{"Wrong library name."};
}

bool ourMethod(const std::string& name) {
  static const std::vector<std::string> orderedMethods = {
      "QLEG1",         "QLEG2",         "QLEG3",         "QLB1",
      "QLB2",          "QLB3",          "QLEG1+DSL",     "QLEG2+DSL",
      "QLEG3+DSL",     "QLB1+DSL",      "QLB2+DSL",      "QLB3+DSL",
      "QLEG1+DSL-FM1", "QLEG2+DSL-FM1", "QLEG3+DSL-FM1", "QLB1+DSL-FM1",
      "QLB2+DSL-FM1",  "QLB3+DSL-FM1",  "QLEG1+DSL-FM2", "QLEG2+DSL-FM2",
      "QLEG3+DSL-FM2", "QLB1+DSL-FM2",  "QLB2+DSL-FM2",  "QLB3+DSL-FM2",
      "QLEG1+DSL-FM3", "QLEG2+DSL-FM3", "QLEG3+DSL-FM3", "QLB1+DSL-FM3",
      "QLB2+DSL-FM3",  "QLB3+DSL-FM3"};
  return std::find(orderedMethods.cbegin(), orderedMethods.cend(), name) !=
         orderedMethods.cend();
}

}  // namespace pipelines
}  // namespace cider
