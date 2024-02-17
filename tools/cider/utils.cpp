// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "utils.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>

using namespace cppast;

namespace cider {
namespace tool {

std::optional<OperatorType> isOperator(const cpp_class& o,
                                       const cpp_member_function& e) {
  return isOperator(o.name().c_str(), e);
}

std::optional<OperatorType> isOperator(const char* classN,
                                       const cppast::cpp_member_function& e) {
  auto name = e.name();
  auto opIt = name.find("operator");
  if (opIt != std::string::npos) {
    name = name.substr(opIt + strlen("operator"));
    if (name.find("=") != std::string::npos &&
        name.find("==") == std::string::npos &&
        name.find("!=") == std::string::npos) {
      std::string className = classN ? classN : "";
      removeScope(className);
      const auto& params = e.parameters();
      for (const auto& param : params) {
        auto paramName = to_string(param.type());
        if (paramName.find(className) != std::string::npos) {
          if (param.type().kind() == cpp_type_kind::reference_t) {
            const auto& reference =
                static_cast<const cpp_reference_type&>(param.type());
            return reference.reference_kind() == cpp_reference::cpp_ref_lvalue
                       ? OperatorType::copyAssignment
                       : OperatorType::moveAssignment;
          }
        }
      }
    }
  }
  return std::nullopt;
}

std::optional<UnaryOpType> isUnOperator(const cppast::cpp_member_function& e) {
  auto name = e.name();
  auto opIt = name.find("operator");
  if (opIt != std::string::npos) {
    name = name.substr(opIt + strlen("operator"));
    if (name.find("-") != std::string::npos &&
        name.find("--") == std::string::npos) {
      const auto& params = e.parameters();
      if (params.empty()) {
        return UnaryOpType::minus;
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

bool isMoveAssignmentOperator(const cppast::cpp_class& o,
                              const cppast::cpp_member_function& e) {
  const auto op = isOperator(o, e);
  return op.has_value() && op.value() == OperatorType::moveAssignment;
}

bool isCopyAssignmentOperator(const cppast::cpp_class& o,
                              const cppast::cpp_member_function& e) {
  const auto op = isOperator(o, e);
  return op.has_value() && op.value() == OperatorType::copyAssignment;
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

bool hasReturnValue(const cpp_conversion_op& e) {
  return hasReturnValue<>(e);
}

bool isUserDefined(const cpp_type& type,
                   const std::string& scope,
                   std::string& name) {
  if (type.kind() == cpp_type_kind::user_defined_t) {
    name = to_string(remove_cv(type));
    replaceScope(scope, name);
    return true;
  } else if (type.kind() == cpp_type_kind::pointer_t) {
    const auto& pointer = static_cast<const cpp_pointer_type&>(type);
    return isUserDefined(pointer.pointee(), scope, name);
  } else if (type.kind() == cpp_type_kind::reference_t) {
    const auto& reference = static_cast<const cpp_reference_type&>(type);
    return isUserDefined(reference.referee(), scope, name);
  } else if (type.kind() == cpp_type_kind::cv_qualified_t) {
    const auto& cvType = static_cast<const cpp_cv_qualified_type&>(type);
    return isUserDefined(cvType.type(), scope, name);
  }
  return false;
}

bool isUserData(const cpp_type& type,
                const std::string& scope,
                const MetadataStorage& metadata) {
  std::string name;
  if (isUserDefined(type, scope, name)) {
    return metadata.classes.find(name) != metadata.classes.end() ||
           metadata.enums.find(name) != metadata.enums.end();
  }
  return false;
}

bool isAggregate(const cpp_type& type,
                 const std::string& scope,
                 const MetadataStorage& metadata) {
  std::string name;
  if (isUserDefined(type, scope, name)) {
    return isAggregate(name, scope, metadata);
  }
  return false;
}

/*
 * An aggregate is an array or a class (Clause 9) with no user-provided
 * constructors (12.1), no brace-or-equal-initializers for non-static
 * data members (9.2), no private or protected non-static data members
 * (Clause 11), no base classes (Clause 10), and no virtual functions (10.3).
 */
bool isAggregate(const ClassMetadata& metadata) {
  return !metadata.hasPrivateFields && !metadata.hasProtectedFields &&
         !metadata.isAbstract;
}

bool isAggregate(std::string name,
                 const std::string& scope,
                 const MetadataStorage& metadata) {
  replaceScope(scope, name);
  auto it = metadata.classes.find(name);
  if (it != metadata.classes.end()) {
    const auto& classMetadata = it->second;
    bool aggregate = isAggregate(classMetadata);
    for (const auto& base : classMetadata.bases) {
      aggregate &= isAggregate(scope + "::" + base, scope, metadata);
    }
    return aggregate;
  }
  return metadata.enums.find(name) != metadata.enums.end();
}

void replaceScope(const std::string& newScope, std::string& value) {
  auto scopePos = value.find_last_of("::");
  if (scopePos != std::string::npos) {
    value = value.substr(scopePos + 1);
  }
  value = newScope + "::" + value;
}

void removeScope(std::string& value) {
  auto scopePos = value.find_last_of("::");
  if (scopePos != std::string::npos) {
    value = value.substr(scopePos + 1);
  }
}

bool isAbstract(const cpp_class& e,
                const std::string& scope,
                const MetadataStorage& metadata) {
  auto enumName = e.name();
  replaceScope(scope, enumName);

  auto it = metadata.classes.find(enumName);
  if (it != metadata.classes.end()) {
    const auto& classMetadata = it->second;
    return classMetadata.isAbstract;
  }
  return false;
}

bool isException(const cppast::cpp_class& e) {
  static const std::unordered_set<std::string> Excpts = {
      "std::exception", "std::logic_error", "std::out_of_range",
      "std::runtime_error"};

  const auto& bases = e.bases();
  for (const auto& one : bases) {
    if (Excpts.find(one.name()) != Excpts.cend()) {
      return true;
    }
  }

  return false;
}

}  // namespace tool
}  // namespace cider
