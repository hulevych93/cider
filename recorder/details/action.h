// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "serialization/serializable.h"

#include "recorder/details/params.h"
#include "recorder/details/sink.h"

#include <optional>
#include <variant>

namespace cider {
namespace recorder {

struct Function final : serialization::SerializableTag {
  std::string name;
  Params params;
  Param retVal;
};

struct ClassMethod final : serialization::SerializableTag {
  void* objectAddress = nullptr;
  Function method;
};

struct ClassDestructor final : serialization::SerializableTag {
  void* objectAddress = nullptr;
};

enum class UnaryOpType { Minus };

struct ClassUnaryOp final : serialization::SerializableTag {
  void* objectAddress = nullptr;
  UnaryOpType opName = UnaryOpType::Minus;
  Param retVal;
};

enum class BinaryOpType { Assignment };

struct ClassBinaryOp final : serialization::SerializableTag {
  void* objectAddress = nullptr;
  BinaryOpType opName = BinaryOpType::Assignment;
  Param param;
};

using Action = std::variant<Function,
                            ClassMethod,
                            ClassBinaryOp,
                            ClassUnaryOp,
                            ClassDestructor>;

Action deepCopy(const Action& action);

void print(std::ostream& os, const Action& action);

bool operator==(const Action& lhs, const Action& rhs);
bool fuzzyEqual(const Action& lhs, const Action& rhs);

bool serialize(const Function& obj, serialization::Serializer& serializer);
bool serialize(const ClassMethod& obj, serialization::Serializer& serializer);
bool serialize(const ClassBinaryOp& obj, serialization::Serializer& serializer);
bool serialize(const ClassUnaryOp& obj, serialization::Serializer& serializer);
bool serialize(const ClassDestructor& obj,
               serialization::Serializer& serializer);

bool deserialize(Function& obj,
                 const serialization::Deserializer& deserializer);
bool deserialize(ClassMethod& obj,
                 const serialization::Deserializer& deserializer);
bool deserialize(ClassBinaryOp& obj,
                 const serialization::Deserializer& deserializer);
bool deserialize(ClassUnaryOp& obj,
                 const serialization::Deserializer& deserializer);
bool deserialize(ClassDestructor& obj,
                 const serialization::Deserializer& deserializer);

namespace details {

template <typename... Types>
struct ParamUnpacker;

template <>
struct ParamUnpacker<> {
  void operator()(Params&) const {}
};

template <typename First, typename... Rest>
struct ParamUnpacker<First, Rest...> {
  void operator()(Params& params, First&& arg, Rest&&... args) const {
    params.emplace_back(makeParam(std::forward<First>(arg)));
    ParamUnpacker<Rest...>{}(params, std::forward<Rest>(args)...);
  }
};

template <typename... ParamsTypes>
Params packParams(ParamsTypes&&... params) {
  Params parameters;
  parameters.reserve(sizeof...(params));
  ParamUnpacker<ParamsTypes...>{}(parameters,
                                  std::forward<ParamsTypes>(params)...);
  return parameters;
}

}  // namespace details

template <typename ReturnType, typename... ParamsTypes>
Action makeAction(const char* function,
                  const ReturnType& retVal,
                  const ParamsTypes&... params) {
  Function obj;
  obj.name = function;
  obj.params = details::packParams(params...);
  obj.retVal = makeParam(retVal);
  return obj;
}  // LCOV_EXCL_LINE

template <typename ReturnType, typename... ParamsTypes>
Action makeAction(const void* object,
                  const char* methodName,
                  const ReturnType& retVal,
                  const ParamsTypes&... params) {
  ClassMethod obj;
  obj.method.name = methodName;
  obj.objectAddress = (void*)object;
  obj.method.params = details::packParams(params...);
  obj.method.retVal = makeParam(retVal);
  return obj;
}  // LCOV_EXCL_LINE

template <typename ParamType>
Action makeAction(const void* object,
                  BinaryOpType type,
                  const ParamType& param) {
  ClassBinaryOp obj;
  obj.objectAddress = (void*)object;
  obj.opName = type;
  obj.param = makeParam(param);
  return obj;
}  // LCOV_EXCL_LINE

template <typename ReturnType>
Action makeAction(const void* object,
                  const ReturnType& retVal,
                  UnaryOpType type) {
  ClassUnaryOp obj;
  obj.objectAddress = (void*)object;
  obj.opName = type;
  obj.retVal = makeParam(retVal);
  return obj;
}  // LCOV_EXCL_LINE

inline Action makeAction(const void* object) {
  ClassDestructor obj;
  obj.objectAddress = (void*)object;
  return obj;
}  // LCOV_EXCL_LINE

struct ActionMutator final {
  explicit ActionMutator(const recorder::IParamMutator& mutator)
      : _mutator(mutator) {}

  bool operator()(recorder::Function& context) {
    bool isMutated = false;
    for (auto& param : context.params) {
      isMutated |= std::visit(_mutator, param);
    }
    return isMutated;
  }

  bool operator()(recorder::ClassMethod& context) {
    bool isMutated = false;
    for (auto& param : context.method.params) {
      isMutated |= std::visit(_mutator, param);
    }
    return isMutated;
  }

  bool operator()(recorder::ClassBinaryOp& context) {
    return std::visit(_mutator, context.param);
  }

  bool operator()(recorder::ClassUnaryOp&) { return false; }
  bool operator()(recorder::ClassDestructor&) { return false; }

  const recorder::IParamMutator& _mutator;
};

}  // namespace recorder
}  // namespace cider

namespace std {

template <>
struct hash<cider::recorder::Function> {
  size_t operator()(const cider::recorder::Function& func) const {
    size_t seed = hash<std::string>{}(func.name);
    hash_combine(seed, func.params);
    hash_combine(seed, func.retVal);
    return seed;
  }
};

template <>
struct hash<cider::recorder::ClassMethod> {
  size_t operator()(const cider::recorder::ClassMethod& method) const {
    size_t seed = hash<decltype(method.method)>{}(method.method);
    return seed;
  }
};

template <>
struct hash<cider::recorder::ClassDestructor> {
  size_t operator()(const cider::recorder::ClassDestructor&) const {
    return 0xce3b9d39;  // Fixed arbitrary value since Nil has no internal state
  }
};

template <>
struct hash<cider::recorder::ClassUnaryOp> {
  size_t operator()(const cider::recorder::ClassUnaryOp& op) const {
    size_t seed = static_cast<int>(op.opName);
    hash_combine(seed, op.retVal);
    return seed;
  }
};

template <>
struct hash<cider::recorder::ClassBinaryOp> {
  size_t operator()(const cider::recorder::ClassBinaryOp& op) const {
    size_t seed = static_cast<int>(op.opName);
    hash_combine(seed, op.param);
    return seed;
  }
};

template <>
struct hash<cider::recorder::Action> {
  size_t operator()(const cider::recorder::Action& action) const {
    return std::visit(
        [](const auto& val) -> size_t {
          size_t hashValue = typeid(std::decay_t<decltype(val)>).hash_code();
          hash_combine(hashValue, val);
          return hashValue;
        },
        action);
  }
};

}  // namespace std
