// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/params.h"
#include "recorder/details/sink.h"

#include <optional>
#include <variant>

namespace cider {
namespace recorder {

struct Function final {
  const char* name = nullptr;
  Params params;
  Param retVal;
  size_t index = 0;
};

struct ClassMethod final {
  const void* objectAddress;
  Function method;
  size_t index = 0;
};

struct ClassDestructor final {
  const void* objectAddress;
  size_t index = 0;
};

enum class UnaryOpType { Minus };

struct ClassUnaryOp final {
  const void* objectAddress;
  UnaryOpType opName = UnaryOpType::Minus;
  Param retVal;
  size_t index = 0;
};

enum class BinaryOpType { Assignment };

struct ClassBinaryOp final {
  const void* objectAddress;
  BinaryOpType opName = BinaryOpType::Assignment;
  Param param;
  size_t index = 0;
};

using Action = std::variant<Function,
                            ClassMethod,
                            ClassBinaryOp,
                            ClassUnaryOp,
                            ClassDestructor>;

Action deepCopy(const Action& action);

void print(std::ostream& os, const Action& action);

bool operator==(const Action& lhs, const Action& rhs);

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
  return Function{function, details::packParams(params...), makeParam(retVal)};
}  // LCOV_EXCL_LINE

template <typename ReturnType, typename... ParamsTypes>
Action makeAction(const void* object,
                  const char* methodName,
                  const ReturnType& retVal,
                  const ParamsTypes&... params) {
  return ClassMethod{object, methodName, details::packParams(params...),
                     makeParam(retVal)};
}  // LCOV_EXCL_LINE

template <typename ParamType>
Action makeAction(const void* object,
                  BinaryOpType type,
                  const ParamType& param) {
  return ClassBinaryOp{object, type, makeParam(param)};
}  // LCOV_EXCL_LINE

template <typename ReturnType>
Action makeAction(const void* object,
                  const ReturnType& retVal,
                  UnaryOpType type) {
  return ClassUnaryOp{object, type, makeParam(retVal)};
}  // LCOV_EXCL_LINE

inline Action makeAction(const void* object) {
  return ClassDestructor{object};
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
