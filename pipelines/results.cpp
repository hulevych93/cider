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

}  // namespace pipelines
}  // namespace cider
