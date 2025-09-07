// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "results.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

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

void printResult(const Result& res) {
  std::cout << "Test Case: " << res.testCaseName << std::endl;

  std::cout << "  Old Actions: " << res.oldActions.size()
            << ", New Actions: " << res.newActions.size() << std::endl;

  std::cout << "  Old Test Exectution Time (mcs): " << res.oldExecutionTimeMcs
            << ", New Test Exectution Time (mcs): " << res.newExecutionTimeMcs
            << std::endl;

  std::cout << "  BR Coverage: "
            << "Old = " << res.oldReport.branchCov.percent
            << " %, New = " << res.newReport.branchCov.percent << " %"
            << std::endl;

  std::cout << " Processing Time (mcs): " << res.timeElapsedMcs << std::endl;
  std::cout << "---------------------------" << std::endl;
}

void printResultsSummary(const Results& results) {
  for (const auto& resIt : results) {
    const auto& methodName = resIt.first;
    std::cout << "=== METHOD: " << methodName << " ===" << std::endl;

    for (const auto& res : resIt.second.entries) {
      printResult(res);
    }
  }
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
      "QLEG1",     "QLEG2",    "QLEG3",     "QLB1",
      "QLB2",      "QLB3",     "QLEG1+DSL", "QLEG2+DSL",
      "QLEG3+DSL", "QLB1+DSL", "QLB2+DSL",  "QLB3+DSL"};
  return std::find(orderedMethods.cbegin(), orderedMethods.cend(), name) !=
         orderedMethods.cend();
}

}  // namespace pipelines
}  // namespace cider
