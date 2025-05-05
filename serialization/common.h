// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

template <class T>
struct is_trivially_serializable
    : std::integral_constant<bool,
                             std::is_object<T>::value &&
                                 std::is_standard_layout<T>::value &&
                                 std::alignment_of<T>::value == 1> {};

namespace cider {
namespace variant_details {

template <typename V>
using creator_ = std::function<void(V&)>;

template <typename V>
using constructors_ = std::vector<creator_<V>>;

template <typename T, typename V>
void construct_variant_(V& v) {
  v = T{};
}

template <typename V>
void default_construct_variant_(V& v) {
  throw std::out_of_range("Variant index out of bounds");
}

template <typename Variant, typename Tuple, std::size_t... Is>
constructors_<Variant> build_constructors_impl(std::index_sequence<Is...>) {
  return constructors_<Variant>{
      &construct_variant_<std::tuple_element_t<Is, Tuple>, Variant>...};
}

template <typename Variant, typename Tuple>
constructors_<Variant> build_constructors() {
  constexpr std::size_t N = std::tuple_size_v<Tuple>;
  return build_constructors_impl<Variant, Tuple>(std::make_index_sequence<N>{});
}
}  // namespace variant_details

template <typename... Ts>
void makeVariant(std::int32_t which, std::variant<Ts...>& object) {
  using Variant = std::variant<Ts...>;
  using Tuple = std::tuple<Ts...>;

  auto constructors = variant_details::build_constructors<Variant, Tuple>();
  if (which < 0 || static_cast<std::size_t>(which) >= constructors.size()) {
    variant_details::default_construct_variant_<Variant>(object);
  } else {
    constructors[which](object);
  }
}

template <typename T>
constexpr size_t type_key() {
#if defined(__clang__) || defined(__GNUC__)
  constexpr auto sig = __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
  constexpr auto sig = __FUNCSIG__;
#else
#error Unsupported compiler
#endif

  size_t hash = 5381;
  for (auto p = sig; *p; ++p)
    hash = ((hash << 5) + hash) + static_cast<unsigned char>(*p);
  return hash;
}

}  // namespace cider
