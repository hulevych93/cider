#pragma once

#include "params.h"
#include "sink.h"

#include <optional>
#include <variant>

namespace gunit {
namespace recorder {

struct FreeFunctionCall final {
  const char* functionName = nullptr;
  Params params;
  bool hasReturnValue = false;
};

struct ClassConstructorCall final {
  const char* className;
  void* objectAddress;
  Params params;
};

struct ClassMethodCall final {
  void* objectAddress;
  const char* methodName = nullptr;
  Params params;
  bool hasReturnValue = false;
};

struct ClassMethodTag {};

using Action =
    std::variant<FreeFunctionCall, ClassConstructorCall, ClassMethodCall>;

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
  return FreeFunctionCall{
      function, details::packParams(std::forward<ParamsTypes>(params)...)};
}

template <typename... ParamsTypes>
Action makeAction(void* object,
                  const char* className,
                  ParamsTypes&&... params) {
  return ClassConstructorCall{
      className, object,
      details::packParams(std::forward<ParamsTypes>(params)...)};
}

template <typename... ParamsTypes>
Action makeAction(void* object,
                  const char* methodName,
                  ClassMethodTag,
                  ParamsTypes&&... params) {
  return ClassMethodCall{
      object, methodName,
      details::packParams(std::forward<ParamsTypes>(params)...)};
}

}  // namespace recorder
}  // namespace gunit
