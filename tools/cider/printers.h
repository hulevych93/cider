#pragma once

#include <iostream>

#include <cppast/cppast_fwd.hpp>
#include <cppast/detail/intrusive_list.hpp>

#include "metadata.h"

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace cider {
namespace tool {

void printNamespace(std::ostream& os,
                    const std::string& scope,
                    const bool enter);

void printFunctionDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_function& e,
                       const namespaces_stack& stack,
                       const char* scopeName,
                       const char* className,
                       const bool declaration = false);

void printFunctionDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_member_function& e,
                       const namespaces_stack& stack,
                       const char* scopeName,
                       const char* className,
                       const bool declaration = false);

void printFunctionBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_function& e,
                       const namespaces_stack& stack);

void printFunctionBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_class& cl,
                       const cppast::cpp_member_function& e,
                       const namespaces_stack& stack);

void printConversionOpBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_class& cl,
                       const cppast::cpp_conversion_op& e,
                       const namespaces_stack& stack);

void printConversionOpDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_conversion_op& e,
                       const namespaces_stack& stack,
                       const char* scopeName,
                       const bool declaration = false);

void printGeneratedMethods(std::ostream& os,
                           const MetadataStorage& metadata,
                           const cppast::cpp_class& e,
                           const namespaces_stack& stack,
                           const bool definition);

void printConstructorDecl(std::ostream& os,
                          const MetadataStorage& metadata,
                          const cppast::cpp_constructor& e,
                          const namespaces_stack& stack,
                          const bool definition = false);

void printBaseClassesConstructors(
    std::ostream& os,
    const MetadataStorage& metadata,
    const cppast::detail::iteratable_intrusive_list<cppast::cpp_base_class>&
        bases,
    const namespaces_stack& stack);

void printVariableDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_member_variable& e,
                       const namespaces_stack& stack);

void printConstructorBody(
    std::ostream& os,
    const MetadataStorage& metadata,
    const cppast::cpp_constructor& e,
    const cppast::detail::iteratable_intrusive_list<cppast::cpp_base_class>&
        bases,
    const namespaces_stack& stack);

void printClassDecl(std::ostream& os,
                    const MetadataStorage& metadata,
                    const cppast::cpp_class& e);

void printClassDef(std::ostream& os,
                   const MetadataStorage& metadata,
                   const cppast::cpp_class& e,
                   const namespaces_stack& stack,
                   const bool enter);

void printEnum(std::ostream& os, const cppast::cpp_enum& e, const bool enter);

void printEnumValue(std::ostream& os, const cppast::cpp_enum_value& e);

void printHeader(std::ostream& os,
                 const MetadataStorage& metadata,
                 const std::string& genScope,
                 const cppast::cpp_file& file);

void printSource(std::ostream& os,
                 const MetadataStorage& metadata,
                 const std::string& genScope,
                 const std::string& outputFile,
                 const cppast::cpp_file& file);

}  // namespace tool
}  // namespace cider
