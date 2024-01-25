#pragma once

#include <cppast/cppast_fwd.hpp>

#include "metadata.h"

namespace cider {
namespace tool {

enum OperatorType {
  copyAssignment,
  moveAssignment
};

std::optional<OperatorType> isOperator(const char* name, const cppast::cpp_member_function& e);
std::optional<OperatorType> isOperator(const cppast::cpp_class& o, const cppast::cpp_member_function& e);

bool isMoveContructor(const cppast::cpp_constructor& e);

bool isCopyContructor(const cppast::cpp_constructor& e);

bool isMoveAssignmentOperator(const cppast::cpp_class& o, const cppast::cpp_member_function& e);

bool isCopyAssignmentOperator(const cppast::cpp_class& o, const cppast::cpp_member_function& e);

bool hasReturnValue(const cppast::cpp_function& e);

bool hasReturnValue(const cppast::cpp_member_function& e);

bool isUserDefined(const cppast::cpp_type& type,
                   const std::string& scope,
                   std::string& name);

bool isUserData(const cppast::cpp_type& type,
                const std::string& scope,
                const MetadataStorage& metadata);

bool isAggregate(const cppast::cpp_type& type,
                 const std::string& scope,
                 const MetadataStorage& metadata);

bool isAggregate(std::string name,
                 const std::string& scope,
                 const MetadataStorage& metadata);

bool isAbstract(const cppast::cpp_class& e,
                const std::string& scope,
                const MetadataStorage& metadata);

bool isException(const cppast::cpp_class& e);

void replaceScope(const std::string& newScope, std::string& value);

void removeScope(std::string& value);

void handleFile(ast_handler& handler,
                const cppast::cpp_file& file,
                bool onlyPublic = true);

}  // namespace tool
}  // namespace cider
