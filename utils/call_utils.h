#pragma once

#include <memory>
#include <type_traits>

namespace cider {

template <typename Type,
          typename ImplType,
          std::enable_if_t<!std::is_aggregate_v<Type>, void*> = nullptr>
auto* getCallee(ImplType* arg) {
  return (arg->_impl.get());
}

template <typename Type,
          typename ImplType,
          std::enable_if_t<!std::is_aggregate_v<Type>, void*> = nullptr>
const auto* getCallee(const ImplType* arg) {
  return (arg->_impl.get());
}

template <typename Type,
          typename ImplType,
          std::enable_if_t<std::is_aggregate_v<Type>, void*> = nullptr>
auto* getCallee(ImplType* arg) {
  return (Type*)arg;
}

template <typename DesiredType, typename ResultType, typename ImplType>
auto getImpl(ResultType& result, ImplType* callee) {
  using ResultTypePure = std::decay_t<ResultType>;
  static_assert(std::is_same_v<DesiredType, ResultTypePure>, "same types");

  using CalleeType = decltype(*callee->_impl.get());
  if constexpr (std::is_same_v<std::decay_t<CalleeType>, ResultType>) {
    if (callee->_impl.get() == std::addressof(result)) {
      return callee->_impl;
    }
    return std::make_shared<ResultType>(*callee->_impl.get());
  }
  return std::make_shared<DesiredType>(result);
}

}  // namespace cider
