#include "utils.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>

using namespace cppast;

namespace gunit {
namespace tool {

std::optional<OperatorType> isOperator(const cpp_member_function& e) {
  if (e.name().find("operator==") != std::string::npos) {
    return OperatorType::Equals;
  } else if (e.name().find("operator!=") != std::string::npos) {
    return OperatorType::NotEquals;
  } else if (e.name().find("operator=") != std::string::npos) {
    const auto& params = e.parameters();
    for (const auto& param : params) {
      if (param.type().kind() == cpp_type_kind::reference_t) {
        const auto& reference =
            static_cast<const cpp_reference_type&>(param.type());
        return reference.reference_kind() == cpp_reference::cpp_ref_lvalue
                   ? OperatorType::copyAssignment
                   : OperatorType::moveAssignment;
      }
    }
  }
  return std::nullopt;
}

bool isCopyMoveContructor(const cpp_constructor& e, const cpp_reference kind) {
  const auto& params = e.parameters();
  if (params.begin() == params.end()) {
    return false;
  }
  if (++params.begin() != params.end()) {
    return false;
  }
  const auto& param = *params.begin();
  if (param.type().kind() == cpp_type_kind::reference_t) {
    const auto& reference =
        static_cast<const cpp_reference_type&>(param.type());
    if (to_string(reference).find(e.name()) != std::string::npos) {
      return reference.reference_kind() == kind;
    }
  }
  return false;
}

bool isMoveContructor(const cpp_constructor& e) {
  return isCopyMoveContructor(e, cpp_reference::cpp_ref_rvalue);
}

bool isCopyContructor(const cppast::cpp_constructor& e) {
  return isCopyMoveContructor(e, cpp_reference::cpp_ref_lvalue);
}

bool isMoveAssignmentOperator(const cppast::cpp_member_function& e) {
  return isOperator(e).value_or(OperatorType::undefined) ==
         OperatorType::moveAssignment;
}

bool isCopyAssignmentOperator(const cppast::cpp_member_function& e) {
  return isOperator(e).value_or(OperatorType::undefined) ==
         OperatorType::copyAssignment;
}

template <typename FunctionType>
bool hasReturnValue(const FunctionType& e) {
  const auto& type = e.return_type();
  if (type.kind() == cpp_type_kind::builtin_t) {
    const auto& buildInType = static_cast<const cpp_builtin_type&>(type);
    return buildInType.builtin_type_kind() != cpp_builtin_type_kind::cpp_void;
  }
  return true;
}

bool hasReturnValue(const cpp_function& e) {
  return hasReturnValue<>(e);
}

bool hasReturnValue(const cpp_member_function& e) {
  return hasReturnValue<>(e);
}

bool isUserDefined(const cpp_type& type, std::string& name) {
  if (type.kind() == cpp_type_kind::user_defined_t) {
    name = to_string(remove_cv(type));
    return true;
  } else if (type.kind() == cpp_type_kind::pointer_t) {
    const auto& pointer = static_cast<const cpp_pointer_type&>(type);
    return isUserDefined(pointer.pointee(), name);
  } else if (type.kind() == cpp_type_kind::reference_t) {
    const auto& reference = static_cast<const cpp_reference_type&>(type);
    return isUserDefined(reference.referee(), name);
  } else if (type.kind() == cpp_type_kind::cv_qualified_t) {
    const auto& cvType = static_cast<const cpp_cv_qualified_type&>(type);
    return isUserDefined(cvType.type(), name);
  }
  return false;
}

bool isUserData(const cpp_type& type, const MetadataStorage& metadata) {
  std::string name;
  if (isUserDefined(type, name)) {
    return metadata.classes.find(name) != metadata.classes.end() ||
           metadata.enums.find(name) != metadata.enums.end();
  }
  return false;
}

bool isAggregate(const cpp_type& type, const MetadataStorage& metadata) {
  std::string name;
  if (isUserDefined(type, name)) {
    return isAggregate(name, metadata);
  }
  return false;
}

bool isAggregate(const std::string& name, const MetadataStorage& metadata) {
  auto it = metadata.classes.find(name);
  if (it != metadata.classes.end()) {
    const auto& classMetadata = it->second;
    return !classMetadata.hasAnyMethods;
  }
  return metadata.enums.find(name) != metadata.enums.end();
}

bool hasImpl(const cpp_type& type,
             const MetadataStorage& metadata,
             std::string& name) {
  if (isUserDefined(type, name)) {
    auto it = metadata.classes.find(name);
    if (it != metadata.classes.end()) {
      const auto& classMetadata = it->second;
      return classMetadata.hasAnyMethods;
    }
  }
  return false;
}

void replaceScope(const std::string& newScope, std::string& value) {
  auto scopePos = value.find_last_of("::");
  if (scopePos != std::string::npos) {
    value = value.substr(scopePos + 1);
    value = newScope + "::" + value;
  }
}

bool isAbstract(const cpp_class& e,
                const char* scope,
                const MetadataStorage& metadata) {
  auto it = metadata.classes.find(std::string{scope} + "::" + e.name());
  if (it != metadata.classes.end()) {
    const auto& classMetadata = it->second;
    return classMetadata.isAbstract;
  }
  return false;
}
}  // namespace tool
}  // namespace gunit
