#pragma once

#include <type_traits>

namespace cider {

template <typename Type, typename OtherType, std::enable_if_t<!std::is_aggregate_v<Type>, void*> = nullptr>
decltype(auto) getCallee(OtherType* arg) {
  return (arg->_impl.get());
}

template <typename Type, typename OtherType, std::enable_if_t<std::is_aggregate_v<Type>, void*> = nullptr>
decltype(auto) getCallee(OtherType* arg) {
  return (Type*)arg;
}

}  // namespace cider
