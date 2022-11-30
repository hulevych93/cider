#pragma once

#include "recorder/details/params.h"
#include "recorder/details/sink.h"

#include <optional>
#include <variant>

namespace gunit {
namespace recorder {

struct Function final {
  const char* name = nullptr;
  Params params;
  Param retVal;
};

struct ClassMethod final {
  const void* objectAddress;
  Function method;
};

enum class BinaryOpType { Assignment };

struct ClassBinaryOp final {
  const void* objectAddress;
  BinaryOpType opName = BinaryOpType::Assignment;
  Param param;
};

using Action = std::variant<Function, ClassMethod, ClassBinaryOp>;

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
                  ReturnType&& retVal,
                  ParamsTypes&&... params) {
  return Function{function,
                  details::packParams(std::forward<ParamsTypes>(params)...),
                  makeParam(std::forward<ReturnType>(retVal))};
}  // LCOV_EXCL_LINE

template <typename ReturnType, typename... ParamsTypes>
Action makeAction(const void* object,
                  const char* methodName,
                  ReturnType&& retVal,
                  ParamsTypes&&... params) {
  return ClassMethod{object, methodName,
                     details::packParams(std::forward<ParamsTypes>(params)...),
                     makeParam(std::forward<ReturnType>(retVal))};
}  // LCOV_EXCL_LINE

template <typename ParamType>
Action makeAction(const void* object, BinaryOpType type, ParamType&& param) {
  return ClassBinaryOp{object, type, makeParam(std::forward<ParamType>(param))};
}  // LCOV_EXCL_LINE

}  // namespace recorder
}  // namespace gunit
