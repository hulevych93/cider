#pragma once

#include <iostream>
#include <stack>
#include <string>

namespace gunit {
namespace tool {

struct namespaces_stack final {
  void operator()(std::ostream& os);

  void push(std::ostream& os, const std::string& scope);
  void pop(std::ostream& os);
  const char* top() const;

 private:
  std::stack<std::string> m_namespaces;
  bool m_inside = false;
};

}  // namespace tool
}  // namespace gunit
