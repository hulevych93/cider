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

struct copy_tag final {};
struct ref_tag final {};

template <typename DesiredType, typename ResultType, typename ImplType>
auto getImpl(ResultType& result, ImplType* callee, ref_tag) {
  using ResultTypePure = std::decay_t<ResultType>;
  static_assert(std::is_same_v<DesiredType, ResultTypePure>, "same types");

  using CalleeType = decltype(*callee->_impl.get());
  if constexpr (std::is_same_v<std::decay_t<CalleeType>, ResultType>) {
    if (callee->_impl.get() == std::addressof(result)) {
      return callee->_impl;
    }
  }
  return std::shared_ptr<DesiredType>((ResultTypePure*)std::addressof(result),
                                      [](void*) {});
}

template <typename DesiredType, typename ResultType, typename ImplType>
auto getImpl(ResultType& result, ImplType*, copy_tag) {
  return std::make_shared<DesiredType>(result);
}

}  // namespace cider
