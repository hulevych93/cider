#include "namespaces_stack.h"

#include "printers.h"

#include <assert.h>

namespace gunit {
namespace tool {

namespaces_stack::namespaces_stack(std::string generatorScope)
    : m_genScope(std::move(generatorScope)) {}

void namespaces_stack::operator()(std::ostream& os) {
  if (!m_inside) {
    os << "namespace " << m_genScope << " {\n\n";
    m_inside = true;
  }
}

void namespaces_stack::push(std::ostream& os, const std::string& scope) {
  m_namespaces.emplace_back(scope);
  assert(!m_inside);
  printNamespace(os, scope, true);
}

void namespaces_stack::pop(std::ostream& os) {
  if (m_inside) {
    os << "} // namespace " << m_genScope << std::endl;
    m_inside = false;
  }
  assert(!m_namespaces.empty());
  printNamespace(os, m_namespaces.back(), false);
  m_namespaces.pop_back();
}

const char* namespaces_stack::top() const {
  return !m_namespaces.empty() ? m_namespaces.back().c_str() : nullptr;
}

std::string namespaces_stack::scope() const {
  std::string scope;
  auto first = true;
  for (const auto& space : m_namespaces) {
    if (!first) {
      scope += "::";
    }
    first = false;
    scope += space;
  }
  return scope;
}

const std::string& namespaces_stack::genScope() const {
  return m_genScope;
}

}  // namespace tool
}  // namespace gunit
