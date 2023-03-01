#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace gunit {
namespace tool {

struct namespaces_stack final {
  void operator()(std::ostream& os);

  void push(std::ostream& os, const std::string& scope);
  void pop(std::ostream& os);
  const char* top() const;
  std::string scope() const;

 private:
  std::vector<std::string> m_namespaces;
  bool m_inside = false;
};

}  // namespace tool
}  // namespace gunit
