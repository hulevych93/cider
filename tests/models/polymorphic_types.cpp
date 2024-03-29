// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "polymorphic_types.h"

namespace cider {
namespace models {

StringInterface::StringInterface(const char* str) : _impl(str) {}
bool StringInterface::isEmpty() const {
  return _impl.empty();
}

OtherStringInterface::OtherStringInterface(bool empty)
    : StringInterface(empty ? "" : "abc") {}

SomeInterface* makeSomeInterface(const char* str) {
  return new StringInterface(str);
}

}  // namespace models
}  // namespace cider
