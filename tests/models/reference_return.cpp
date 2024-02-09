// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "reference_return.h"

namespace cider {
namespace models {

RefStruct& RefStruct::self() {
  return *this;
}

RefStruct RefStruct::copy() {
  return *this;
}

int RefStruct::inc() {
  return ++field;
}

}  // namespace models
}  // namespace cider
