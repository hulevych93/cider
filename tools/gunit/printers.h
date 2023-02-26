#pragma once

#include <iostream>

#include <cppast/cppast_fwd.hpp>
#include <cppast/detail/intrusive_list.hpp>

namespace gunit {
namespace tool {

constexpr const auto* GeneratedNamespaceName = "gunit_hook";

void printType(std::ostream& os,
               const cppast::cpp_entity_index& idx,
               const cppast::cpp_type& type);

void printParams(std::ostream& os,
                 const cppast::cpp_entity_index& idx,
                 const cppast::detail::iteratable_intrusive_list<
                     cppast::cpp_function_parameter>& params,
                 const bool includeTypes = false);

void printNamespace(std::ostream& os,
                    const std::string& scope,
                    const bool enter);

void printFunctionNotify(std::ostream& os,
                         const bool member,
                         const bool hasNoReturn,
                         const bool hasNoArgs);

void printFunctionDecl(std::ostream& os,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_function& e,
                       const char* scope,
                       const bool semicolon = false);

void printFunctionDecl(std::ostream& os,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_member_function& e,
                       const char* scope,
                       const bool semicolon = false);

void printFunctionBody(std::ostream& os,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_function& e,
                       const char* scope = nullptr);

void printFunctionBody(std::ostream& os,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_member_function& e,
                       const char* scope = nullptr);

void printConstructorDecl(std::ostream& os,
                          const cppast::cpp_entity_index& idx,
                          const cppast::cpp_constructor& e,
                          const bool definition = false);

void printConstructorNotify(std::ostream& os, const bool hasNoArgs);

void printBaseClassesConstructors(
    std::ostream& os,
    const cppast::detail::iteratable_intrusive_list<cppast::cpp_base_class>&
        bases,
    const char* scope);

void printConstructorBody(std::ostream& os,
                          const cppast::cpp_entity_index& idx,
                          const cppast::cpp_constructor& e,
                          const char* scope);

void printBaseClasses(
    std::ostream& os,
    const cppast::detail::iteratable_intrusive_list<cppast::cpp_base_class>&
        bases);

void printClass(std::ostream& os,
                const cppast::cpp_class& e,
                const char* scope,
                const bool isPrivate,
                const bool enter);

}  // namespace tool
}  // namespace gunit
