#pragma once

#include <cppast/cppast_fwd.hpp>

#include "metadata.h"

namespace cider {
namespace tool {

enum OperatorType {
  undefined,
  copyAssignment,
  moveAssignment,
  Equals,
  NotEquals
};

std::optional<OperatorType> isOperator(const cppast::cpp_member_function& e);

bool isMoveContructor(const cppast::cpp_constructor& e);

bool isCopyContructor(const cppast::cpp_constructor& e);

bool isMoveAssignmentOperator(const cppast::cpp_member_function& e);

bool isCopyAssignmentOperator(const cppast::cpp_member_function& e);

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
bool isAggregate(const std::string& name, const MetadataStorage& metadata);

bool isAbstract(const cppast::cpp_class& e,
                const std::string& scope,
                const MetadataStorage& metadata);

bool hasImpl(const cppast::cpp_type& type,
             const std::string& scope,
             const MetadataStorage& metadata,
             std::string& name);

void replaceScope(const std::string& newScope, std::string& value);

void removeScope(std::string& value);

void handleFile(ast_handler& handler, const cppast::cpp_file& file);

}  // namespace tool
}  // namespace cider
