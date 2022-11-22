#pragma once

#include "params.h"
#include "user_data.h"

#include <optional>
#include <variant>

namespace gunit {
namespace recorder {

struct FreeFunctionCall final {
  const char* function = nullptr;
  Params params;
};

using Action = std::variant<FreeFunctionCall>;

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
Action makeFreeFunctionCall(const char* function, ParamsTypes&&... params) {
  return FreeFunctionCall{
      function, details::packParams(std::forward<ParamsTypes>(params)...)};
}

}  // namespace recorder
}  // namespace gunit
