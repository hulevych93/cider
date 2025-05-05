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
  unsigned int index = 0;
};

struct ClassMethod final : serialization::SerializableTag {
  void* objectAddress = nullptr;
  Function method;
  unsigned int index = 0;
};

struct ClassDestructor final : serialization::SerializableTag {
  void* objectAddress = nullptr;
  unsigned int index = 0;
};

enum class UnaryOpType { Minus };

struct ClassUnaryOp final : serialization::SerializableTag {
  void* objectAddress = nullptr;
  UnaryOpType opName = UnaryOpType::Minus;
  Param retVal;
  unsigned int index = 0;
};

enum class BinaryOpType { Assignment };

struct ClassBinaryOp final : serialization::SerializableTag {
  void* objectAddress = nullptr;
  BinaryOpType opName = BinaryOpType::Assignment;
  Param param;
  unsigned int index = 0;
};

using Action = std::variant<Function,
                            ClassMethod,
                            ClassBinaryOp,
                            ClassUnaryOp,
                            ClassDestructor>;

Action deepCopy(const Action& action);

void print(std::ostream& os, const Action& action);

bool operator==(const Action& lhs, const Action& rhs);

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

}  // namespace recorder
}  // namespace cider

namespace std {

template <>
struct hash<cider::recorder::Nil> {
  size_t operator()(const cider::recorder::Nil&) const noexcept {
    return 0x9e3779b9;  // Fixed arbitrary value since Nil has no internal state
  }
};

template <typename T>
inline void hash_combine(size_t& seed, const T& val) {
  seed ^= hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <>
struct hash<cider::recorder::Param> {
  size_t operator()(const cider::recorder::Param& param) const {
    return std::visit(
        [](const auto& val) -> size_t {
          return hash<std::decay_t<decltype(val)>>{}(val);
        },
        param);
  }
};

template <>
struct hash<cider::recorder::Params> {
  size_t operator()(const cider::recorder::Params& params) const {
    size_t seed = 0;
    for (const auto& param : params) {
      hash_combine(seed, param);
    }
    return seed;
  }
};

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
    size_t seed = reinterpret_cast<size_t>(method.objectAddress);
    hash_combine(seed, method.method);
    return seed;
  }
};

template <>
struct hash<cider::recorder::ClassDestructor> {
  size_t operator()(const cider::recorder::ClassDestructor& destructor) const {
    return reinterpret_cast<size_t>(destructor.objectAddress);
  }
};

template <>
struct hash<cider::recorder::ClassUnaryOp> {
  size_t operator()(const cider::recorder::ClassUnaryOp& op) const {
    size_t seed = reinterpret_cast<size_t>(op.objectAddress);
    hash_combine(seed, static_cast<int>(op.opName));
    hash_combine(seed, op.retVal);
    return seed;
  }
};

template <>
struct hash<cider::recorder::ClassBinaryOp> {
  size_t operator()(const cider::recorder::ClassBinaryOp& op) const {
    size_t seed = reinterpret_cast<size_t>(op.objectAddress);
    hash_combine(seed, static_cast<int>(op.opName));
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
