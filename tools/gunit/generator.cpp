#include "generator.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_file.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>
#include <cppast/visitor.hpp>

#include "printers.h"
#include "utils.h"

using namespace cppast;

namespace gunit {
namespace tool {

generator::generator(std::ostream& out,
                     std::string genScope,
                     const MetadataStorage& metadata)
    : m_namespaces(std::move(genScope)), m_out(out), m_metadata(metadata) {}

void generator::handleClass(const cpp_class& e, const bool enter) {
  if (enter) {
    m_class = std::addressof(e);
  } else {
    m_class = nullptr;
  }
}

void generator::handleNamespace(const cpp_entity& e, const bool enter) {
  if (enter) {
    m_namespaces.push(m_out, e.name());
  } else {
    m_namespaces.pop(m_out);
  }
}

void header_generator::handleClass(const cpp_class& e, const bool enter) {
  generator::handleClass(e, enter);

  if (enter) {
    m_namespaces(m_out);
  }

  if (e.class_kind() == cpp_class_kind::class_t) {
    printClass(m_out, m_metadata, e, m_namespaces.genScope().c_str(),
               m_namespaces.top(), enter);
    if (enter && !isAbstract(e, m_namespaces.scope().c_str(), m_metadata)) {
      printGeneratedMethods(m_out, m_metadata, e, m_namespaces.scope().c_str(),
                            false);
    }
  } else {
    printStruct(m_out, m_metadata, e, enter);
  }
}

void header_generator::handleConstructor(const cpp_constructor& e) {
  printConstructorDecl(m_out, m_metadata, e, m_namespaces.genScope().c_str(),
                       false);
}

void header_generator::handleFreeFunction(const cpp_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_metadata, e, m_namespaces.genScope().c_str(),
                    nullptr, true);

  m_out << std::endl;
}

void header_generator::handleMemberFunction(const cpp_member_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_metadata, e, m_namespaces.genScope().c_str(),
                    nullptr, true);

  m_out << std::endl;
}

void header_generator::handleMemberVariable(
    const cppast::cpp_member_variable& e) {
  m_namespaces(m_out);

  printVariableDecl(m_out, m_metadata, e, m_namespaces.genScope().c_str());
}

void source_generator::handleClass(const cppast::cpp_class& e, bool enter) {
  generator::handleClass(e, enter);

  if (enter && e.class_kind() == cpp_class_kind::class_t &&
      !isAbstract(e, m_namespaces.scope().c_str(), m_metadata)) {
    m_namespaces(m_out);
    printGeneratedMethods(m_out, m_metadata, e, m_namespaces.scope().c_str(),
                          true);
    m_out << std::endl;
  }
}

void source_generator::handleConstructor(const cpp_constructor& e) {
  m_namespaces(m_out);

  printConstructorDecl(m_out, m_metadata, e, m_namespaces.genScope().c_str(),
                       true);
  printBaseClassesConstructors(m_out, m_metadata, m_class->bases(),
                               m_namespaces.genScope().c_str(),
                               m_namespaces.top());
  printConstructorBody(m_out, m_metadata, e, m_class->bases(),
                       m_namespaces.top());

  m_out << std::endl;
}

void source_generator::handleFreeFunction(const cpp_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_metadata, e, m_namespaces.genScope().c_str(),
                    nullptr, false);
  printFunctionBody(m_out, m_metadata, e, m_namespaces.genScope().c_str(),
                    m_namespaces.top());

  m_out << std::endl;
}

void source_generator::handleMemberFunction(const cpp_member_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_metadata, e, m_namespaces.genScope().c_str(),
                    m_class->name().c_str(), false);
  printFunctionBody(m_out, m_metadata, e, m_namespaces.genScope().c_str());

  m_out << std::endl;
}

void handleFile(ast_handler& handler, const cpp_file& file) {
  handler.handleFile(file, true);
  cppast::visit(file, [&](const cpp_entity& e, visitor_info info) {
    const auto enter = info.event == visitor_info::container_entity_enter;
    if (info.access != cpp_access_specifier_kind::cpp_public) {
      // skip not public entities
      return;
    }
    switch (e.kind()) {
      case cpp_entity_kind::namespace_t:
        handler.handleNamespace(e, enter);
        break;
      case ::cpp_entity_kind::function_t:
        handler.handleFreeFunction(static_cast<const cpp_function&>(e));
        break;
      case ::cpp_entity_kind::class_t:
        handler.handleClass(static_cast<const cpp_class&>(e), enter);
        break;
      case ::cpp_entity_kind::constructor_t:
        handler.handleConstructor(static_cast<const cpp_constructor&>(e));
        break;
      case ::cpp_entity_kind::member_function_t:
        handler.handleMemberFunction(
            static_cast<const cpp_member_function&>(e));
        break;
      case ::cpp_entity_kind::member_variable_t:
        handler.handleMemberVariable(
            static_cast<const cpp_member_variable&>(e));
      default:
        break;
    }
  });
  handler.handleFile(file, false);
}

}  // namespace tool
}  // namespace gunit
