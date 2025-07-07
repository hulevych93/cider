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
  unsigned long timeElapsedMs = 0;
};

double getMinimizationEfficency(const Result& result);

using Results = std::unordered_map<std::string, std::vector<Result>>;

bool serialize(const Result& obj, serialization::Serializer& serializer);
bool deserialize(Result& obj, const serialization::Deserializer& deserializer);

}  // namespace pipelines
}  // namespace cider
