// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "action.h"

namespace cider {
namespace recorder {

namespace {

bool operator==(const Function& lhs, const Function& rhs) {
  return std::strcmp(lhs.name, rhs.name) == 0 && lhs.params == rhs.params &&
         lhs.retVal == rhs.retVal;
}

bool operator==(const ClassMethod& lhs, const ClassMethod& rhs) {
  return lhs.objectAddress == rhs.objectAddress && lhs.method == rhs.method;
}

bool operator==(const ClassDestructor& lhs, const ClassDestructor& rhs) {
  return lhs.objectAddress == rhs.objectAddress;
}

bool operator==(const ClassUnaryOp& lhs, const ClassUnaryOp& rhs) {
  return lhs.objectAddress == rhs.objectAddress && lhs.opName == rhs.opName &&
         lhs.retVal == rhs.retVal;
}

bool operator==(const ClassBinaryOp& lhs, const ClassBinaryOp& rhs) {
  return lhs.objectAddress == rhs.objectAddress && lhs.opName == rhs.opName &&
         lhs.param == rhs.param;
}
}  // namespace

bool operator==(const Action& lhs, const Action& rhs) {
  if (lhs.index() != rhs.index())
    return false;

  return std::visit(
      [](const auto& lhsVal, const auto& rhsVal) {
        using LhsType = std::decay_t<decltype(lhsVal)>;
        using RhsType = std::decay_t<decltype(rhsVal)>;
        if constexpr (std::is_same_v<LhsType, RhsType>) {
          return lhsVal == rhsVal;
        } else {
          return false;
        }
      },
      lhs, rhs);
}

}  // namespace recorder
}  // namespace cider
