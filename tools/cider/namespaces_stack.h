// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace cider {
namespace tool {

struct namespaces_stack final {
  namespaces_stack() = default;
  explicit namespaces_stack(std::string generatorScope);

  void operator()(std::ostream& os);

  void push(std::ostream& os, const std::string& scope);
  void pop(std::ostream& os);

  std::string genScope() const;
  std::string nativeScope() const;
  std::string top() const;

 private:
  std::vector<std::string> m_namespaces;
  bool m_inside = false;
  std::string m_genScope;
};

}  // namespace tool
}  // namespace cider
