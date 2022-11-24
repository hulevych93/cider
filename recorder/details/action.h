#pragma once

#include "params.h"
#include "sink.h"

#include <optional>
#include <variant>

namespace gunit {
namespace recorder {

struct FreeFunction final {
  const char* functionName = nullptr;
  Params params;
  bool hasReturnValue = false;
};

struct ClassConstructor final {
  const char* className;
  void* objectAddress;
  Params params;
};

struct ClassMethod final {
  void* objectAddress;
  const char* methodName = nullptr;
  Params params;
  bool hasReturnValue = false;
};

enum class BinaryOpType { Assignment };

struct ClassBinaryOp final {
  void* objectAddress;
  BinaryOpType opName = BinaryOpType::Assignment;
  Param param;
  bool hasReturnValue = false;
};

struct ClassMethodTag {};

using Action =
    std::variant<FreeFunction, ClassConstructor, ClassMethod, ClassBinaryOp>;

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

template <typename... ParamsTypes>
Action makeAction(const char* function, ParamsTypes&&... params) {
  return FreeFunction{
      function, details::packParams(std::forward<ParamsTypes>(params)...)};
}

template <typename... ParamsTypes>
Action makeAction(void* object,
                  const char* className,
                  ParamsTypes&&... params) {
  return ClassConstructor{
      className, object,
      details::packParams(std::forward<ParamsTypes>(params)...)};
}

template <typename... ParamsTypes>
Action makeAction(void* object,
                  const char* methodName,
                  ClassMethodTag,
                  ParamsTypes&&... params) {
  return ClassMethod{object, methodName,
                     details::packParams(std::forward<ParamsTypes>(params)...)};
}

template <typename ParamType>
Action makeAction(void* object, BinaryOpType type, ParamType&& param) {
  return ClassBinaryOp{object, type, makeParam(std::forward<ParamType>(param))};
}

}  // namespace recorder
}  // namespace gunit
