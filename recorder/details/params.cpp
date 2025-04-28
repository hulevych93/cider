// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "params.h"

namespace cider {
namespace recorder {

Param deepCopy(const Param& param) {
  return std::visit(
      [](auto&& value) -> Param {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, Nil> || std::is_same_v<T, bool> ||
                      std::is_same_v<T, double> ||
                      std::is_same_v<T, std::string> ||
                      std::is_same_v<T, std::wstring> ||
                      std::is_same_v<T, IntegerType>) {
          return value;
        } else if constexpr (std::is_same_v<T, UserDataValueParamPtr> ||
                             std::is_same_v<T, UserDataReferenceParamPtr>) {
          return value->deepCopy();
        } else {
          static_assert(!sizeof(T), "Unsupported type in Param");
        }
      },
      param);
}

void print(std::ostream& os, const cider::recorder::Param& param) {
  std::visit(
      [&os](auto&& value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, Nil>) {
          os << "Nil";
        } else if constexpr (std::is_same_v<T, bool> ||
                             std::is_same_v<T, double> ||
                             std::is_same_v<T, std::string>) {
          os << value;
        } else if constexpr (std::is_same_v<T, std::wstring>) {
          os << "Nan";
        } else if constexpr (std::is_same_v<T, IntegerType>) {
          std::visit([&os](auto&& integer) { os << integer; }, value);
        } else if constexpr (std::is_same_v<T, UserDataValueParamPtr> ||
                             std::is_same_v<T, UserDataReferenceParamPtr>) {
          value->print(os);
        } else {
          static_assert(!sizeof(T), "Unsupported type in Param");
        }
      },
      param);
}

}  // namespace recorder
}  // namespace cider
