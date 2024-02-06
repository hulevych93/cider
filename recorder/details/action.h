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
};

struct ClassMethod final {
  const void* objectAddress;
  Function method;
};

struct ClassDestructor final {
    const void* objectAddress;
};

enum class UnaryOpType { Minus };

struct ClassUnaryOp final {
  const void* objectAddress;
  UnaryOpType opName = UnaryOpType::Minus;
  Param retVal;
};

enum class BinaryOpType { Assignment };

struct ClassBinaryOp final {
  const void* objectAddress;
  BinaryOpType opName = BinaryOpType::Assignment;
  Param param;
};

using Action = std::variant<Function, ClassMethod, ClassBinaryOp, ClassUnaryOp, ClassDestructor>;

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
