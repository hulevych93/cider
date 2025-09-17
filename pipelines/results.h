// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT
#pragma once

#include "coverage/coverage.h"

#include "coverage/cfg_coverage.h"
#include "coverage/gcov_coverage.h"

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

#include <unordered_map>

namespace cider {
namespace pipelines {

struct Input final {
  recorder::Actions actions;
  std::string testOrLibName;
};

struct Result final : serialization::SerializableTag {
  std::string testCaseName;
  recorder::Actions oldActions;
  recorder::Actions newActions;
  gcov_coverage::CoverageReport oldReport;
  gcov_coverage::CoverageReport newReport;
  cfg_coverage::Coverage oldCfgReport;
  cfg_coverage::Coverage newCgfReport;
  unsigned long timeElapsedMcs = 0;
  unsigned long oldExecutionTimeMcs = 0;
  unsigned long newExecutionTimeMcs = 0;
  std::optional<unsigned long> coverageReachedLength;
};

struct MethodResults final : serialization::SerializableTag {
  std::vector<Result> entries;
};

using Results = std::unordered_map<std::string, MethodResults>;

bool serialize(const Result& obj, serialization::Serializer& serializer);
bool deserialize(Result& obj, const serialization::Deserializer& deserializer);

bool serialize(const MethodResults& obj, serialization::Serializer& serializer);
bool deserialize(MethodResults& obj,
                 const serialization::Deserializer& deserializer);

void printResult(const std::string& methodName,
                 const std::string& libName,
                 const Result& result);

int getDataSize(const std::string& libName);

double getOldCov(const std::string& libName,
                 const gcov_coverage::CoverageReport& report);

bool ourMethod(const std::string& name);

template <typename F, typename P>
void processBest(P* plot,
                 const std::string& methodName,
                 const std::string& libName,
                 const cider::Cmd& cmd,
                 const std::vector<Result>& results,
                 int max,
                 F&& func) {
  std::vector<Result> bestResults = results;

  if (ourMethod(methodName)) {
    std::sort(bestResults.begin(), bestResults.end(),
              [](const auto& l, const auto& r) {
                return l.newReport.branchCov.percent >
                       r.newReport.branchCov.percent;
              });
  }

  int i = 1;
  for (const auto& rs : bestResults) {
    func(plot, methodName, libName, cmd, rs);
    ++i;
    if (i > max) {
      break;
    }
  }
}

}  // namespace pipelines
}  // namespace cider
