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

Action deepCopy(const Action& action) {
  return std::visit(
      [](auto&& value) -> Action {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, Function>) {
          Function copy;
          copy.name = value.name;
          copy.retVal = deepCopy(value.retVal);
          copy.params = deepCopy(value.params);
          return copy;
        } else if constexpr (std::is_same_v<T, ClassMethod>) {
          ClassMethod copy;
          copy.objectAddress = value.objectAddress;
          copy.method.name = value.method.name;
          copy.method.retVal = deepCopy(value.method.retVal);
          copy.method.params = deepCopy(value.method.params);
          return copy;
        } else if constexpr (std::is_same_v<T, ClassBinaryOp>) {
          ClassBinaryOp copy;
          copy.objectAddress = value.objectAddress;
          copy.opName = value.opName;
          copy.param = deepCopy(value.param);
          return copy;
        } else if constexpr (std::is_same_v<T, ClassUnaryOp>) {
          ClassUnaryOp copy;
          copy.objectAddress = value.objectAddress;
          copy.opName = value.opName;
          copy.retVal = deepCopy(value.retVal);
          return copy;
        } else if constexpr (std::is_same_v<T, ClassDestructor>) {
          ClassDestructor copy;
          copy.objectAddress = value.objectAddress;
          return copy;
        } else {
          static_assert(!sizeof(T), "Unsupported Action type");
        }
      },
      action);
}

void print(std::ostream& os, const Action& action) {
  std::visit(
      [&os](auto&& value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, cider::recorder::Function>) {
          print(os, value.retVal);
          os << " " << value.name << "(";
          print(os, value.params);
          os << ")";
        } else if constexpr (std::is_same_v<T, cider::recorder::ClassMethod>) {
          print(os, value.method.retVal);
          os << " "
             << "obj@" << value.objectAddress << "->" << value.method.name
             << "(";
          print(os, value.method.params);
          os << ")";
        } else if constexpr (std::is_same_v<T,
                                            cider::recorder::ClassBinaryOp>) {
          std::string opStr =
              (value.opName == cider::recorder::BinaryOpType::Assignment) ? "="
                                                                          : "?";
          os << "obj@" << value.objectAddress << " " << opStr << " ";
          print(os, value.param);
        } else if constexpr (std::is_same_v<T, cider::recorder::ClassUnaryOp>) {
          std::string opStr =
              (value.opName == cider::recorder::UnaryOpType::Minus) ? "-" : "?";
          os << opStr << "obj@" << value.objectAddress;
        } else if constexpr (std::is_same_v<T,
                                            cider::recorder::ClassDestructor>) {
          os << "destroy(obj@" +
                    std::to_string(
                        reinterpret_cast<uintptr_t>(value.objectAddress)) +
                    ")";
        } else {
          static_assert(!sizeof(T), "Unsupported Action type");
        }
      },
      action);
}

}  // namespace recorder
}  // namespace cider
