#pragma once

#include <memory>
#include <type_traits>

namespace cider {

template <typename Type,
          typename ImplType,
          std::enable_if_t<!std::is_aggregate_v<Type>, void*> = nullptr>
decltype(auto) getCallee(ImplType* arg) {
  return (arg->_impl.get());
}

template <typename Type,
          typename ImplType,
          std::enable_if_t<std::is_aggregate_v<Type>, void*> = nullptr>
decltype(auto) getCallee(ImplType* arg) {
  return (Type*)arg;
}

template <typename Type, typename ImplType>
auto getImpl(Type& result, ImplType* callee) {
  if (callee->_impl.get() == std::addressof(result)) {
    return callee->_impl;
  }

  return std::make_shared<Type>(*callee->_impl.get());
}

}  // namespace cider
