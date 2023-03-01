#include "generator.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_file.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>
#include <cppast/visitor.hpp>

#include <fstream>

#include "printers.h"

using namespace cppast;

namespace gunit {
namespace tool {

generator::generator(std::ostream& out,
                     const MetadataStorage& metadata,
                     const cpp_entity_index& idx)
    : m_out(out), m_metadata(metadata), m_idx(idx) {}

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

header_generator::header_generator(std::ostream& out,
                                   const MetadataStorage& metadata,
                                   const cpp_entity_index& idx)
    : generator(out, metadata, idx) {}

void header_generator::handleClass(const cpp_class& e, const bool enter) {
  generator::handleClass(e, enter);

  if (enter) {
    m_namespaces(m_out);
  }

  if (e.class_kind() == cpp_class_kind::class_t) {
    printClass(m_out, m_metadata, e, m_namespaces.top(), enter);
  } else {
    printStruct(m_out, m_metadata, e, enter);
  }
}

void header_generator::handleConstructor(const cpp_constructor& e) {
  printConstructorDecl(m_out, m_metadata, m_idx, e, false);
}

void header_generator::handleFreeFunction(const cpp_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_metadata, m_idx, e, nullptr, true);

  m_out << std::endl;
}

void header_generator::handleMemberFunction(const cpp_member_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_metadata, m_idx, e, nullptr, true);

  m_out << std::endl;
}

void header_generator::handleMemberVariable(
    const cppast::cpp_member_variable& e) {
  m_namespaces(m_out);

  printVariableDecl(m_out, m_metadata, m_idx, e);
}

source_generator::source_generator(std::ostream& out,
                                   const MetadataStorage& metadata,
                                   const cpp_entity_index& idx)
    : generator(out, metadata, idx) {}

void source_generator::handleConstructor(const cpp_constructor& e) {
  m_namespaces(m_out);

  printConstructorDecl(m_out, m_metadata, m_idx, e, true);
  printBaseClassesConstructors(m_out, m_metadata, m_class->bases(),
                               m_namespaces.top());
  printConstructorBody(m_out, m_metadata, m_idx, e, m_namespaces.top());

  m_out << std::endl;
}

void source_generator::handleFreeFunction(const cpp_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_metadata, m_idx, e, nullptr, false);
  printFunctionBody(m_out, m_metadata, m_idx, e, m_namespaces.top());

  m_out << std::endl;
}

void source_generator::handleMemberFunction(const cpp_member_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_metadata, m_idx, e, m_class->name().c_str(),
                    false);
  printFunctionBody(m_out, m_metadata, m_idx, e);

  m_out << std::endl;
}

constexpr const auto* GeneratedFileHeader =
    R"(/* This file is auto-generated by gunit tool */)";

void process_file(ast_handler& handler, const cpp_file& file) {
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
}

void printHeader(const std::string& outputFile,
                 const MetadataStorage& metadata,
                 const cppast::cpp_entity_index& idx,
                 const cppast::cpp_file& file) {
  std::ofstream headerStream(outputFile + ".h");
  header_generator header(headerStream, metadata, idx);

  headerStream << GeneratedFileHeader << "\n\n";
  headerStream << "#pragma once\n\n";
  headerStream << "#include <memory>\n\n";
  headerStream << "#include \"" << file.name() << "\"\n\n";

  process_file(header, file);
}

void printSource(const std::string& outputFile,
                 const MetadataStorage& metadata,
                 const cppast::cpp_entity_index& idx,
                 const cppast::cpp_file& file) {
  std::ofstream sourceStream(outputFile + ".cpp");
  source_generator source(sourceStream, metadata, idx);

  sourceStream << GeneratedFileHeader << "\n\n";
  sourceStream << "#include \"" << outputFile + ".h"
               << "\"\n\n";
  sourceStream << "#include <recorder/actions_observer.h>\n\n";

  process_file(source, file);
}

}  // namespace tool
}  // namespace gunit
