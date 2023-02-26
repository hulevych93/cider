#include "namespaces_stack.h"

#include "printers.h"

#include <assert.h>

namespace gunit {
namespace tool {

void namespaces_stack::operator()(std::ostream& os) {
  if (!m_inside) {
    os << "namespace " << GeneratedNamespaceName << " {\n\n";
    m_inside = true;
  }
}

void namespaces_stack::push(std::ostream& os, const std::string& scope) {
  m_namespaces.push(scope);
  assert(!m_inside);
  printNamespace(os, m_namespaces.top(), true);
}

void namespaces_stack::pop(std::ostream& os) {
  if (m_inside) {
    os << "} // namespace " << GeneratedNamespaceName << std::endl;
    m_inside = false;
  }
  printNamespace(os, m_namespaces.top(), false);
  m_namespaces.pop();
}

const char* namespaces_stack::top() const {
  return !m_namespaces.empty() ? m_namespaces.top().c_str() : nullptr;
}

}  // namespace tool
}  // namespace gunit
