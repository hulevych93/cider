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
  const auto coeff =
      static_cast<double>(result.newActions.size()) / result.oldActions.size();
  assert(coeff <= 1.0f);
  return 1.0f - static_cast<double>(result.newActions.size()) /
                    result.oldActions.size();
}

bool serialize(const Result& obj, serialization::Serializer& serializer) {
  serializer << obj.testCaseName;
  serializer << obj.timeElapsedMs;
  serializer << obj.oldActions;
  serializer << obj.newActions;
  serializer << obj.oldReport;
  serializer << obj.newReport;
  serializer << obj.oldCfgReport;
  serializer << obj.newCgfReport;
  return true;
}

bool deserialize(Result& obj, const serialization::Deserializer& deserializer) {
  deserializer >> obj.testCaseName;
  deserializer >> obj.timeElapsedMs;
  deserializer >> obj.oldActions;
  deserializer >> obj.newActions;
  deserializer >> obj.oldReport;
  deserializer >> obj.newReport;
  deserializer >> obj.oldCfgReport;
  deserializer >> obj.newCgfReport;
  return true;
}

void printResultsSummary(const Results& results) {
  for (const auto& [libName, resultList] : results) {
    std::cout << "=== Library: " << libName << " ===" << std::endl;
    for (const auto& res : resultList) {
      std::cout << "Test Case: " << res.testCaseName << std::endl;

      std::cout << "  Old Actions: " << res.oldActions.size()
                << ", New Actions: " << res.newActions.size() << std::endl;

      std::cout << "  BR Coverage: "
                << "Old = " << res.oldReport.branchCov.percent
                << " %, New = " << res.newReport.branchCov.percent << " %"
                << std::endl;

      std::cout << "  Time Elapsed (ms): " << res.timeElapsedMs << std::endl;

      double efficiency = getMinimizationEfficency(res);
      std::cout << "  Minimization Efficiency: " << std::fixed
                << std::setprecision(2) << efficiency << std::endl;

      std::cout << "---------------------------" << std::endl;
    }
  }
}

}  // namespace pipelines
}  // namespace cider
