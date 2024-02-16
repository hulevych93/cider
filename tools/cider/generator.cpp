// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "generator.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_enum.hpp>
#include <cppast/cpp_file.hpp>
#include <cppast/cpp_friend.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_template.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>
#include <cppast/cpp_variable.hpp>
#include "cppast/cpp_type_alias.hpp"

#include <cppast/visitor.hpp>

#include "printers.h"
#include "utils.h"

using namespace cppast;

namespace cider {
namespace tool {

generator::generator(std::ostream& out,
                     std::string genScope,
                     const MetadataStorage& metadata)
    : m_namespaces(std::move(genScope)), m_out(out), m_metadata(metadata) {}

void generator::handleClass(const cpp_class& e,
                            cppast::cpp_access_specifier_kind /*kind*/,
                            const bool enter) {
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

void header_generator::handleClass(const cpp_class& e,
                                   cppast::cpp_access_specifier_kind kind,
                                   const bool enter) {
  generator::handleClass(e, kind, enter);

  if (enter) {
    m_namespaces(m_out);
  }

  if (isException(e)) {
    return;
  }

  if (e.is_declaration()) {
    if (enter) {
      printClassDecl(m_out, m_metadata, e);
    }
    return;
  }

  printClassDef(m_out, m_metadata, e, m_namespaces, enter);

  if (enter && !isAbstract(e, m_namespaces.nativeScope(), m_metadata) &&
      !isAggregate(e.name(), m_namespaces.nativeScope(), m_metadata)) {
    printGeneratedMethods(m_out, m_metadata, e, m_namespaces, false);
  }
}

void header_generator::handleConstructor(
    const cpp_constructor& e,
    cppast::cpp_access_specifier_kind /*kind*/) {
  printConstructorDecl(m_out, m_metadata, e, m_namespaces, false);
}

void header_generator::handleFreeFunction(const cpp_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_metadata, e, m_namespaces, nullptr, nullptr, true);

  m_out << std::endl;
}

void header_generator::handleEnumValue(const cppast::cpp_enum_value& e) {
  printEnumValue(m_out, e);
}

void header_generator::handleEnum(const cppast::cpp_enum& e, const bool enter) {
  if (enter) {
    m_namespaces(m_out);
  }

  printEnum(m_out, e, enter);
}

void header_generator::handleDestructor(
    const cppast::cpp_destructor& e,
    cppast::cpp_access_specifier_kind /*kind*/) {
  m_namespaces(m_out);

  printDestructorDecl(m_out, m_metadata, e);

  m_out << std::endl;
}

void header_generator::handleMemberFunction(
    const cpp_member_function& e,
    cppast::cpp_access_specifier_kind kind) {
  m_namespaces(m_out);

  if (kind != cppast::cpp_access_specifier_kind::cpp_public) {
    return;
  }

  printFunctionDecl(m_out, m_metadata, e, m_namespaces, nullptr,
                    m_class->name().c_str(), true);

  m_out << std::endl;
}

void header_generator::handleConversionOperator(
    const cppast::cpp_conversion_op& e,
    cppast::cpp_access_specifier_kind kind) {
  m_namespaces(m_out);

  printConversionOpDecl(m_out, m_metadata, e, m_namespaces, nullptr, true);

  m_out << std::endl;
}

void header_generator::handleMemberVariable(
    const cppast::cpp_member_variable& e,
    cppast::cpp_access_specifier_kind /*kind*/) {
  if (isAggregate(m_class->name(), m_namespaces.nativeScope(), m_metadata)) {
    m_namespaces(m_out);
    printVariableDecl(m_out, m_metadata, e, m_namespaces);
  }
}

void header_generator::handleVariable(const cppast::cpp_variable& e) {
  printVariableDecl(m_out, m_metadata, e, m_namespaces);
}

void header_generator::handleAlias(const cppast::cpp_type_alias& e) {
  m_namespaces(m_out);

  printAliasDecl(m_out, m_metadata, e, m_namespaces);
}

void header_generator::handleFriend(const cppast::cpp_friend& e) {
  const auto& entity = e.entity();
  if (entity.has_value()) {
    const auto* func =
        dynamic_cast<const cpp_function*>(std::addressof(entity.value()));
    if (func != nullptr) {
      m_out << "friend ";
      handleFreeFunction(*func);
    }
  }
}

void source_generator::handleClass(const cppast::cpp_class& e,
                                   cppast::cpp_access_specifier_kind kind,
                                   bool enter) {
  generator::handleClass(e, kind, enter);

  if (e.is_declaration()) {
    return;
  }

  if (enter && !isAbstract(e, m_namespaces.nativeScope(), m_metadata) &&
      !isAggregate(e.name(), m_namespaces.nativeScope(), m_metadata)) {
    m_namespaces(m_out);
    printGeneratedMethods(m_out, m_metadata, e, m_namespaces, true);
    m_out << std::endl;
  }
}

void source_generator::handleConstructor(
    const cpp_constructor& e,
    cppast::cpp_access_specifier_kind /*kind*/) {
  m_namespaces(m_out);

  printConstructorDecl(m_out, m_metadata, e, m_namespaces, true);
  printBaseClassesConstructors(m_out, m_metadata, m_class->bases(),
                               m_namespaces);
  printConstructorBody(m_out, m_metadata, e, m_class->bases(), m_namespaces);

  m_out << std::endl;
}

void source_generator::handleDestructor(
    const cppast::cpp_destructor& e,
    cppast::cpp_access_specifier_kind /*kind*/) {
  m_namespaces(m_out);

  printDestructorDef(m_out, m_metadata, e);

  m_out << std::endl;
}

void source_generator::handleFreeFunction(const cpp_function& e) {
  m_namespaces(m_out);

  printFunctionDecl(m_out, m_metadata, e, m_namespaces, nullptr, nullptr,
                    false);
  printFunctionBody(m_out, m_metadata, e, m_namespaces);

  m_out << std::endl;
}

void source_generator::handleMemberFunction(
    const cpp_member_function& e,
    cppast::cpp_access_specifier_kind kind) {
  m_namespaces(m_out);

  if (kind != cppast::cpp_access_specifier_kind::cpp_public) {
    return;
  }

  printFunctionDecl(m_out, m_metadata, e, m_namespaces, m_class->name().c_str(),
                    m_class->name().c_str(), false);
  printFunctionBody(m_out, m_metadata, *m_class, e, m_namespaces);

  m_out << std::endl;
}

void source_generator::handleConversionOperator(
    const cppast::cpp_conversion_op& e,
    cppast::cpp_access_specifier_kind kind) {
  m_namespaces(m_out);

  printConversionOpDecl(m_out, m_metadata, e, m_namespaces,
                        m_class->name().c_str(), false);
  printConversionOpBody(m_out, m_metadata, *m_class, e, m_namespaces);

  m_out << std::endl;
}

void source_generator::handleFriend(const cppast::cpp_friend& e) {
  const auto& entity = e.entity();
  if (entity.has_value()) {
    const auto* func =
        dynamic_cast<const cpp_function*>(std::addressof(entity.value()));
    if (func != nullptr) {
      m_namespaces(m_out);

      printFunctionDecl(m_out, m_metadata, *func, m_namespaces, nullptr,
                        nullptr, false);
      printFunctionBody(m_out, m_metadata, *func, m_namespaces, true);

      m_out << std::endl;
    }
  }
}

void handleFile(ast_handler& handler,
                const cpp_file& file,
                const bool onlyPublic) {
  handler.handleFile(file, true);

  auto isTemplate = false;
  cppast::visit(file, [&](const cpp_entity& e, visitor_info info) {
    const auto enter = info.event == visitor_info::container_entity_enter;
    if (onlyPublic && info.access != cpp_access_specifier_kind::cpp_public) {
      // skip not public entities
      return;
    }

    if (e.kind() == cpp_entity_kind::function_template_t ||
        e.kind() == cpp_entity_kind::class_template_t) {
      isTemplate = enter;
    }

    const auto comment = e.comment();
    if (comment) {
      auto value = comment.value();
      if (value.find("@cider_ignore") != std::string::npos) {
        return;
      }
    }

    if (isTemplate) {
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
        handler.handleClass(static_cast<const cpp_class&>(e), info.access,
                            enter);
        break;
      case ::cpp_entity_kind::constructor_t:
        handler.handleConstructor(static_cast<const cpp_constructor&>(e),
                                  info.access);
        break;
      case ::cpp_entity_kind::member_function_t:
        handler.handleMemberFunction(static_cast<const cpp_member_function&>(e),
                                     info.access);
        break;
      case ::cpp_entity_kind::conversion_op_t:
        handler.handleConversionOperator(
            static_cast<const cpp_conversion_op&>(e), info.access);
        break;
      case ::cpp_entity_kind::member_variable_t:
        handler.handleMemberVariable(static_cast<const cpp_member_variable&>(e),
                                     info.access);
        break;
      case ::cpp_entity_kind::destructor_t:
        handler.handleDestructor(static_cast<const cpp_destructor&>(e),
                                 info.access);
        break;
      case ::cpp_entity_kind::enum_t:
        handler.handleEnum(static_cast<const cpp_enum&>(e), enter);
        break;
      case ::cpp_entity_kind::friend_t:
        handler.handleFriend(static_cast<const cpp_friend&>(e));
        break;
      case ::cpp_entity_kind::enum_value_t:
        handler.handleEnumValue(static_cast<const cpp_enum_value&>(e));
        break;
      case ::cpp_entity_kind::variable_t:
        handler.handleVariable(static_cast<const cpp_variable&>(e));
        break;
      case ::cpp_entity_kind::type_alias_t:
        handler.handleAlias(static_cast<const cpp_type_alias&>(e));
        break;
      default:
        break;
    }
  });
  handler.handleFile(file, false);
}

}  // namespace tool
}  // namespace cider
