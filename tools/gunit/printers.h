#pragma once

#include <iostream>

#include <cppast/cppast_fwd.hpp>
#include <cppast/detail/intrusive_list.hpp>

#include "metadata.h"

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace gunit {
namespace tool {

constexpr const auto* GeneratedNamespaceName = "gunit_hook";

void printNamespace(std::ostream& os,
                    const std::string& scope,
                    const bool enter);

void printFunctionDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_function& e,
                       const char* scope,
                       const bool semicolon = false);

void printFunctionDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_member_function& e,
                       const char* scope,
                       const bool semicolon = false);

void printFunctionBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_function& e,
                       const char* scope = nullptr);

void printFunctionBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_member_function& e,
                       const char* scope = nullptr);

void printConstructorDecl(std::ostream& os,
                          const MetadataStorage& metadata,
                          const cppast::cpp_entity_index& idx,
                          const cppast::cpp_constructor& e,
                          const bool definition = false);

void printBaseClassesConstructors(
    std::ostream& os,
    const MetadataStorage& metadata,
    const cppast::detail::iteratable_intrusive_list<cppast::cpp_base_class>&
        bases,
    const char* scope);

void printVariableDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_member_variable& e);

void printConstructorBody(std::ostream& os,
                          const MetadataStorage& metadata,
                          const cppast::cpp_entity_index& idx,
                          const cppast::cpp_constructor& e,
                          const char* scope);

void printClass(std::ostream& os,
                const MetadataStorage& metadata,
                const cppast::cpp_class& e,
                const char* scope,
                const bool enter);

void printStruct(std::ostream& os,
                 const MetadataStorage& metadata,
                 const cppast::cpp_class& e,
                 const bool enter);

}  // namespace tool
}  // namespace gunit