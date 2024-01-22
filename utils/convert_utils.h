#pragma once

#include <type_traits>

namespace cider {

template <typename Type, typename OtherType>
decltype(auto) convert_in(OtherType const& arg) {
  return *(std::decay_t<Type>*)&arg;
}

template <typename Type, typename OtherType>
decltype(auto) convert_in(OtherType* arg) {
  return (std::remove_pointer_t<Type>*)arg;
}

template <typename Type, typename OtherType>
Type convert_out(OtherType arg) {
  return *(Type*)&arg;
}

}  // namespace cider
