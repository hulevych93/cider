// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

namespace cider {
namespace models {

class DerivedClass;

class BaseClass {
 public:
  BaseClass(const char* str);

  DerivedClass get();
  BaseClass get() const;

  std::string sayHello() const;

 private:
  std::string _impl;
};

class DerivedClass final : public BaseClass {
 public:
  DerivedClass(const char* str);

  std::string sayGoodbye(int times) const;

 private:
  std::string _impl;
};

}  // namespace models
}  // namespace cider
