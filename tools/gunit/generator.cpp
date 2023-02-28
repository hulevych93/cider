#include "generator.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>

#include <assert.h>

#include "printers.h"

using namespace cppast;

namespace gunit {
namespace tool {

generator::generator(std::ostream& out, const cpp_entity_index& idx)
    : m_out(out), m_idx(idx) {}

void generator::handleAccess(const cpp_entity& e) {
  assert(e.kind() == cpp_entity_kind::access_specifier_t);
}

void generator::handleClass(const cpp_class& e, const bool enter) {
  if (enter) {
    m_class = std::addressof(e);
  } else {
    m_class = nullptr;
  }
}

void generator::handleMemberVariable(const cppast::cpp_member_variable&) {
  // nothing to do
}

void generator::handleNamespace(const cpp_entity& e, const bool enter) {
  if (enter) {
    m_namespaces.push(m_out, e.name());
  } else {
    m_namespaces.pop(m_out);
  }
}

header_generator::header_generator(std::ostream& out,
                                   const cpp_entity_index& idx)
    : generator(out, idx) {}

void header_generator::handleClass(const cpp_class& e, const bool enter) {
  generator::handleClass(e, enter);

  if (enter) {
    m_access = e.class_kind() == cpp_class_kind::class_t ? "private" : "public";

    m_namespaces(m_out);
  }

  if (e.class_kind() == cpp_class_kind::class_t) {
    printClass(m_out, e, m_namespaces.top(), m_access == "private", enter);
  } else {
    printStruct(m_out, e, enter);
  }
}

void header_generator::handleConstructor(const cpp_constructor& e) {
  assert(m_class->name() == e.name());
  printConstructorDecl(m_out, m_idx, e, false);
}

void header_generator::handleAccess(const cpp_entity& e) {
  assert(e.kind() == cpp_entity_kind::access_specifier_t);
  if (m_access != e.name()) {
    m_access = e.name();
    m_out << e.name() << ":\n";
  }
}

void header_generator::handleFreeFunction(const cpp_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_idx, e, nullptr, true);

  m_out << std::endl;
}

void header_generator::handleMemberFunction(const cpp_member_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_idx, e, nullptr, true);

  m_out << std::endl;
}

void header_generator::handleMemberVariable(
    const cppast::cpp_member_variable& e) {
  if (m_access != "public") {
    return;
  }

  m_namespaces(m_out);

  printVariableDecl(m_out, m_idx, e);

  m_out << std::endl;
}

source_generator::source_generator(std::ostream& out,
                                   const cpp_entity_index& idx)
    : generator(out, idx) {}

void source_generator::handleConstructor(const cpp_constructor& e) {
  assert(m_class->name() == e.name());
  m_namespaces(m_out);

  printConstructorDecl(m_out, m_idx, e, true);
  printBaseClassesConstructors(m_out, m_class->bases(), m_namespaces.top());
  printConstructorBody(m_out, m_idx, e, m_namespaces.top());

  m_out << std::endl;
}

void source_generator::handleFreeFunction(const cpp_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_idx, e, nullptr, false);
  printFunctionBody(m_out, m_idx, e, m_namespaces.top());

  m_out << std::endl;
}

void source_generator::handleMemberFunction(const cpp_member_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_idx, e, m_class->name().c_str(), false);
  printFunctionBody(m_out, m_idx, e);

  m_out << std::endl;
}

}  // namespace tool
}  // namespace gunit
