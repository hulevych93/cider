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

double getMinimizationEfficency(const Result& result) {
  if (result.oldActions.size() == 0)
    return 0.0;
  double coeff = 0.0;
  if (result.coverageReachedLength.has_value()) {
    coeff = static_cast<double>(result.coverageReachedLength.value()) /
            result.oldActions.size();
  } else {
    coeff = static_cast<double>(result.newActions.size()) /
            result.oldActions.size();
  }
  assert(coeff <= 1.0f);
  return 1.0f - static_cast<double>(result.newActions.size()) /
                    result.oldActions.size();
}

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
  serializer << obj.coverageReachedCount;
  serializer << obj.failedCount;
  serializer << obj.sessionsCount;
  serializer << obj.totalTimeElapsedMcs;
#endif
  return true;
}

bool deserialize(MethodResults& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.entries;
#if RESULTS_VERSION_2
  deserializer >> obj.coverageReachedCount;
  deserializer >> obj.failedCount;
  deserializer >> obj.sessionsCount;
  deserializer >> obj.totalTimeElapsedMcs;
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

  double efficiency = getMinimizationEfficency(res);
  std::cout << "  Minimization Efficiency: " << std::fixed
            << std::setprecision(2) << efficiency << std::endl;

  std::cout << "---------------------------" << std::endl;
}

void printResultsSummary(const std::string& method, const Results& results) {
  for (const auto& resIt : results) {
    const auto& methodName = resIt.first;
    if (method == methodName) {
      std::cout << "=== Library: " << methodName << " ===" << std::endl;

      for (const auto& res : resIt.second.entries) {
        printResult(res);
      }
    }
  }
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

}  // namespace pipelines
}  // namespace cider
