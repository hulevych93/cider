// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "results.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cider {
namespace pipelines {

std::ostream& operator<<(std::ostream& os, const BriefResult& br) {
  os << "{";
  os << "methodName: " << br.methodName << ", ";
  os << "testOrLibName: " << br.testOrLibName << ", ";
  os << "oldLines: " << br.oldLines << ", ";
  os << "newLines: " << br.newLines;
  os << "}";
  return os;
}

bool serialize(const Result& obj, serialization::Serializer& serializer) {
  serializer << obj.actions;
  serializer << obj.methodName;
  serializer << obj.testOrLibName;
  return true;
}

bool deserialize(Result& obj, const serialization::Deserializer& deserializer) {
  deserializer >> obj.actions;
  deserializer >> obj.methodName;
  deserializer >> obj.testOrLibName;
  return true;
}

bool serialize(const BriefResult& obj, serialization::Serializer& serializer) {
  serializer << obj.report;
  serializer << obj.oldLines;
  serializer << obj.newLines;
  serializer << obj.methodName;
  serializer << obj.testOrLibName;
  return true;
}

bool deserialize(BriefResult& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.report;
  deserializer >> obj.oldLines;
  deserializer >> obj.newLines;
  deserializer >> obj.methodName;
  deserializer >> obj.testOrLibName;
  return true;
}

bool serialize(const SessionsResult& obj,
               serialization::Serializer& serializer) {
  serializer << obj.libName;
  serializer << obj.sessions;
  return true;
}

bool deserialize(SessionsResult& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.libName;
  deserializer >> obj.sessions;
  return true;
}

}  // namespace pipelines
}  // namespace cider
