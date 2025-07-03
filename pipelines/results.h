// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT
#pragma once

#include "coverage/coverage.h"

#include "coverage/gcov_coverage.h"

#include "metaheuristics/metasearch.h"

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

#include <unordered_map>

namespace cider {
namespace pipelines {

struct Input final {
  recorder::Actions actions;
  std::function<recorder::Actions()> actionsCallback;
  std::string testOrLibName;
};

struct Result final : serialization::SerializableTag {
  recorder::Actions actions;
  std::string methodName;
  std::string testOrLibName;
};

struct BriefResult final : serialization::SerializableTag {
  gcov_coverage::CoverageReport report;
  int oldLines = 0;
  int newLines = 0;
  std::string methodName;
  std::string testOrLibName;
};

std::ostream& operator<<(std::ostream& os, const BriefResult& br);

struct SessionsResult final : serialization::SerializableTag {
  std::vector<recorder::Actions> sessions;
  std::string libName;
};

bool serialize(const Result& obj, serialization::Serializer& serializer);
bool deserialize(Result& obj, const serialization::Deserializer& deserializer);

bool serialize(const BriefResult& obj, serialization::Serializer& serializer);
bool deserialize(BriefResult& obj,
                 const serialization::Deserializer& deserializer);

bool serialize(const SessionsResult& obj,
               serialization::Serializer& serializer);
bool deserialize(SessionsResult& obj,
                 const serialization::Deserializer& deserializer);

using Results = std::vector<Result>;
using BriefResults = std::vector<BriefResult>;
using SessionsResults = std::vector<SessionsResult>;

using ResultsStorage = std::unordered_map<PipelineType, Results>;
using BriefResultStorage = std::unordered_map<PipelineType, BriefResults>;
using SessionsResultStorage = std::unordered_map<PipelineType, SessionsResults>;

}  // namespace pipelines
}  // namespace cider
