// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "action.h"

#include <tlog.h>

namespace cider {
namespace recorder {

namespace {

bool operator==(const Function& lhs, const Function& rhs) {
  return lhs.name == rhs.name && lhs.params == rhs.params &&
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

bool fuzzyEqual(const Function& lhs, const Function& rhs) {
  return lhs.name == rhs.name && fuzzyEqual(lhs.params, rhs.params) &&
         fuzzyEqual(lhs.retVal, rhs.retVal);
}

bool fuzzyEqual(const ClassMethod& lhs, const ClassMethod& rhs) {
  return fuzzyEqual(lhs.method, rhs.method);
}

bool fuzzyEqual(const ClassDestructor&, const ClassDestructor&) {
  return true;
}

bool fuzzyEqual(const ClassUnaryOp& lhs, const ClassUnaryOp& rhs) {
  return lhs.opName == rhs.opName && fuzzyEqual(lhs.retVal, rhs.retVal);
}

bool fuzzyEqual(const ClassBinaryOp& lhs, const ClassBinaryOp& rhs) {
  return lhs.opName == rhs.opName && fuzzyEqual(lhs.param, rhs.param);
}

bool semanticallyEqual(const Function& lhs, const Function& rhs) {
  return lhs.name == rhs.name && lhs.params.size() == rhs.params.size();
}

bool semanticallyEqual(const ClassMethod& lhs, const ClassMethod& rhs) {
  return semanticallyEqual(lhs.method, rhs.method);
}

bool semanticallyEqual(const ClassDestructor&, const ClassDestructor&) {
  return true;
}

bool semanticallyEqual(const ClassUnaryOp& lhs, const ClassUnaryOp& rhs) {
  return lhs.opName == rhs.opName;
}

bool semanticallyEqual(const ClassBinaryOp& lhs, const ClassBinaryOp& rhs) {
  return lhs.opName == rhs.opName;
}

}  // namespace

bool serialize(const Function& obj, serialization::Serializer& serializer) {
  serializer << obj.name;
  serializer << obj.params;
  serializer << obj.retVal;
  return true;
}

bool serialize(const ClassMethod& obj, serialization::Serializer& serializer) {
  serializer << obj.objectAddress;
  serializer << obj.method;
  return true;
}

bool serialize(const ClassDestructor& obj,
               serialization::Serializer& serializer) {
  serializer << obj.objectAddress;
  return true;
}

bool serialize(const ClassUnaryOp& obj, serialization::Serializer& serializer) {
  serializer << obj.objectAddress;
  serializer << obj.opName;
  serializer << obj.retVal;
  return true;
}

bool serialize(const ClassBinaryOp& obj,
               serialization::Serializer& serializer) {
  serializer << obj.objectAddress;
  serializer << obj.opName;
  serializer << obj.param;
  return true;
}

bool deserialize(Function& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.name;
  deserializer >> obj.params;
  deserializer >> obj.retVal;
  return true;
}

bool deserialize(ClassMethod& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.objectAddress;
  deserializer >> obj.method;
  return true;
}

bool deserialize(ClassDestructor& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.objectAddress;
  return true;
}

bool deserialize(ClassUnaryOp& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.objectAddress;
  deserializer >> obj.opName;
  deserializer >> obj.retVal;
  return true;
}

bool deserialize(ClassBinaryOp& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.objectAddress;
  deserializer >> obj.opName;
  deserializer >> obj.param;
  return true;
}

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

bool fuzzyEqual(const Action& lhs, const Action& rhs) {
  if (lhs.index() != rhs.index())
    return false;

  return std::visit(
      [](const auto& lhsVal, const auto& rhsVal) {
        using LhsType = std::decay_t<decltype(lhsVal)>;
        using RhsType = std::decay_t<decltype(rhsVal)>;
        if constexpr (std::is_same_v<LhsType, RhsType>) {
          return fuzzyEqual(lhsVal, rhsVal);
        } else {
          return false;
        }
      },
      lhs, rhs);
}

bool fuzzyEqual(const std::vector<Action>& lhs,
                const std::vector<Action>& rhs) {
  bool result = lhs.size() == rhs.size();
  if (result) {
    auto fIt = lhs.cbegin();
    auto sIt = rhs.cbegin();
    for (; fIt != lhs.cend(); ++fIt, ++sIt) {
      result = fuzzyEqual(*fIt, *sIt);
      if (!result) {
        break;
      }
    }
  }
  return result;
}

bool semanticallyEqual(const Action& lhs, const Action& rhs) {
  if (lhs.index() != rhs.index())
    return false;

  return std::visit(
      [](const auto& lhsVal, const auto& rhsVal) {
        using LhsType = std::decay_t<decltype(lhsVal)>;
        using RhsType = std::decay_t<decltype(rhsVal)>;
        if constexpr (std::is_same_v<LhsType, RhsType>) {
          return semanticallyEqual(lhsVal, rhsVal);
        } else {
          return false;
        }
      },
      lhs, rhs);
}

bool semanticallyEqual(const std::vector<Action>& lhs,
                       const std::vector<Action>& rhs) {
  bool result = lhs.size() == rhs.size();
  if (result) {
    auto fIt = lhs.cbegin();
    auto sIt = rhs.cbegin();
    for (; fIt != lhs.cend(); ++fIt, ++sIt) {
      result = semanticallyEqual(*fIt, *sIt);
      if (!result) {
        break;
      }
    }
  }
  return result;
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

        if constexpr (std::is_same_v<T, Function>) {
          print(os, value.retVal);
          os << " " << value.name << "(";
          print(os, value.params);
          os << ")";
        } else if constexpr (std::is_same_v<T, ClassMethod>) {
          print(os, value.method.retVal);
          os << " "
             << "obj@" << value.objectAddress << "->" << value.method.name
             << "(";
          print(os, value.method.params);
          os << ")";
        } else if constexpr (std::is_same_v<T, ClassBinaryOp>) {
          std::string opStr =
              (value.opName == BinaryOpType::Assignment) ? "=" : "?";
          os << "obj@" << value.objectAddress << " " << opStr << " ";
          print(os, value.param);
        } else if constexpr (std::is_same_v<T, ClassUnaryOp>) {
          std::string opStr = (value.opName == UnaryOpType::Minus) ? "-" : "?";
          os << opStr << "obj@" << value.objectAddress;
        } else if constexpr (std::is_same_v<T, ClassDestructor>) {
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

std::string actionToGenericRepro(const recorder::Action& action) {
  std::stringstream os;
  std::visit(
      [&os](auto&& value) {
        using T = std::decay_t<decltype(value)>;

        auto printParams = [&os](const auto& args) {
          os << "(";
          for (size_t i = 0; i < args.size(); ++i) {
            os << "_";  // assuming `typeName` gives a string like "int"
            if (i + 1 != args.size())
              os << ", ";
          }
          os << ")";
        };

        if constexpr (std::is_same_v<T, cider::recorder::Function>) {
          os << value.name;
          printParams(value.params);  // assuming .arguments is a list of types
        } else if constexpr (std::is_same_v<T, cider::recorder::ClassMethod>) {
          os << "obj@->" << value.method.name;
          printParams(value.method.params);  // nested method inside ClassMethod
        } else if constexpr (std::is_same_v<T,
                                            cider::recorder::ClassBinaryOp>) {
          std::string opStr =
              (value.opName == cider::recorder::BinaryOpType::Assignment) ? "="
                                                                          : "?";
          os << "obj@ " << opStr;
        } else if constexpr (std::is_same_v<T, cider::recorder::ClassUnaryOp>) {
          std::string opStr =
              (value.opName == cider::recorder::UnaryOpType::Minus) ? "-" : "?";
          os << opStr << "obj@";
        } else if constexpr (std::is_same_v<T,
                                            cider::recorder::ClassDestructor>) {
          os << "destroy(obj@)";
          // likely no params for destructor
        } else {
          static_assert(!sizeof(T), "Unsupported Action type");
        }
      },
      action);
  return os.str();
}

std::string actionsToGenericRepro(
    const std::vector<recorder::Action>& actions) {
  if (actions.empty()) {
    return "Empty";
  }
  std::string result;
  for (const auto& action : actions) {
    result += actionToGenericRepro(action) + ';';
  }
  result.pop_back();
  return result;
}

}  // namespace recorder
}  // namespace cider
