#include "printers.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>

#include <cppast/detail/assert.hpp>

using namespace cppast;

namespace gunit {
namespace tool {

constexpr const auto* CatchBlock = R"(} catch (const std::exception&) {
  // ex.what() notify
  throw;
} catch (...) {
  // NOTIFY some expection
  throw;
}
)";

namespace {

template <typename FunctionType>
bool hasReturnValue(const FunctionType& e) {
  const auto& type = e.return_type();
  if (type.kind() == cpp_type_kind::builtin_t) {
    const auto& buildInType = static_cast<const cpp_builtin_type&>(type);
    return buildInType.builtin_type_kind() != cpp_builtin_type_kind::cpp_void;
  }
  return true;
}

}  // namespace

// void eraseScope(std::string& value) {
//     auto scopePos = value.find_last_of("::");
//     if(scopePos != std::string::npos) {
//         value = value.substr(scopePos + 1);
//     }
// }

void printParamType(std::ostream& os,
                    const cpp_entity_index& idx,
                    const cpp_type& type) {
  auto value = to_string(type);
  //    auto scopePos = value.find_last_of("::");
  //    if(scopePos != std::string::npos) {
  //        value = value.substr(scopePos + 1);
  //    }
  os << value;
}

void printParams(
    std::ostream& os,
    const cpp_entity_index& idx,
    const detail::iteratable_intrusive_list<cpp_function_parameter>& params,
    const bool includeTypes) {
  auto first = true;
  for (const auto& param : params) {
    if (!first) {
      os << ", ";
    } else {
      first = false;
    }
    if (includeTypes) {
      printParamType(os, idx, param.type());
      os << " ";
    }
    if (!param.name().empty()) {
      os << param.name();
    }
  }
}

void printNamespace(std::ostream& os,
                    const std::string& scope,
                    const bool enter) {
  if (enter) {
    os << "namespace " << scope << " {\n";
  } else {
    os << "} // namespace " << scope << "\n";
  }
}

void printFunctionNotify(std::ostream& os,
                         const bool member,
                         const bool hasNoReturn,
                         const bool hasNoArgs) {
  os << "GUNIT_NOTIFY_";
  if (member) {
    os << "METHOD";
  } else {
    os << "FREE_FUNCTION";
  }
  if (hasNoArgs) {
    os << "_NO_ARGS";
  }
  if (hasNoReturn) {
    os << "(std::nullopt";
  } else {
    os << "(result";
  }
  if (!hasNoArgs) {
    os << ", ";
  }
}

template <typename FunctionType>
void printFunctionDecl(std::ostream& os,
                       const cpp_entity_index& idx,
                       const FunctionType& e,
                       const char* scope,
                       const bool semicolon) {
  printParamType(os, idx, e.return_type());
  os << " ";
  if (scope != nullptr) {
    os << scope << "::";
  }
  os << e.name() << "(";
  const auto& params = e.parameters();
  printParams(os, idx, params, true);
  os << ")";
  if constexpr (std::is_same_v<cpp_member_function_base, FunctionType>) {
    if (is_const(e.cv_qualifier())) {
      os << " const";
    }
  }
  if (semicolon) {
    os << ";";
  }
  os << "\n";
}

void printFunctionDecl(std::ostream& os,
                       const cpp_entity_index& idx,
                       const cpp_function& e,
                       const char* scope,
                       const bool semicolon) {
  printFunctionDecl<>(os, idx, e, scope, semicolon);
}

void printFunctionDecl(std::ostream& os,
                       const cpp_entity_index& idx,
                       const cpp_member_function& e,
                       const char* scope,
                       const bool semicolon) {
  printFunctionDecl<>(os, idx, e, scope, semicolon);
}

template <typename FunctionType>
void printFunctionBody(std::ostream& os,
                       const cpp_entity_index& idx,
                       const FunctionType& e,
                       const bool member,
                       const char* scope) {
  os << "{\n";
  os << "try {\n";
  const auto hasReturn = hasReturnValue(e);
  if (hasReturn) {
    os << "auto result = ";
  }
  if constexpr (std::is_same_v<cpp_member_function, FunctionType>) {
    os << "_impl->" << e.name() << "(";
  } else {
    os << scope << "::" << e.name() << "(";
  }
  const auto& params = e.parameters();
  printParams(os, idx, params, false);
  os << ");\n";

  printFunctionNotify(os, member, !hasReturn, params.empty());
  printParams(os, idx, params, false);
  os << ");\n";
  if (hasReturn) {
    os << "return result;\n";
  }
  os << CatchBlock << "}\n";
}

void printFunctionBody(std::ostream& os,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_function& e,
                       const char* scope) {
  printFunctionBody<>(os, idx, e, false, scope);
}

void printFunctionBody(std::ostream& os,
                       const cppast::cpp_entity_index& idx,
                       const cppast::cpp_member_function& e,
                       const char* scope) {
  printFunctionBody<>(os, idx, e, true, scope);
}

void printConstructorDecl(std::ostream& os,
                          const cpp_entity_index& idx,
                          const cpp_constructor& e,
                          const bool definition) {
  if (definition) {
    os << e.name() << "::";
  }
  os << e.name() << "(";
  const auto& params = e.parameters();
  printParams(os, idx, params, true);
  os << ")";
  if (!definition) {
    os << ";";
  }
  os << "\n";
}

void printConstructorNotify(std::ostream& os, const bool hasNoArgs) {
  os << "GUNIT_NOTIFY_CONSTRUCTOR";
  if (hasNoArgs) {
    os << "_NO_ARGS";
  }
}

void printBaseClassesConstructors(
    std::ostream& os,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases,
    const char* scope) {
  if (bases.empty()) {
    return;
  }

  os << ": ";
  auto first = true;
  for (const auto& base : bases) {
    if (!first) {
      os << ", ";
    } else {
      first = false;
    }
    os << base.name() << "(std::shared_ptr<" << scope << "::" << base.name()
       << ">())";
  }
}

void printConstructorBody(std::ostream& os,
                          const cpp_entity_index& idx,
                          const cpp_constructor& e,
                          const char* scope) {
  os << "{\n";
  os << "try {\n";
  os << "_impl = std::make_shared<" << scope << "::" << e.name() << ">(";

  const auto& params = e.parameters();
  printParams(os, idx, params, false);
  os << ");\n";

  printConstructorNotify(os, params.empty());
  if (!params.empty()) {
    os << "(";
    printParams(os, idx, params, false);
    os << ");\n";
  }

  os << CatchBlock << "}\n";
}

void printBaseClasses(
    std::ostream& os,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases) {
  if (bases.empty()) {
    return;
  }

  os << ": ";
  auto first = true;
  for (const auto& base : bases) {
    if (!first) {
      os << ", ";
    } else {
      first = false;
    }
    os << to_string(base.access_specifier()) << " " << base.name() << " ";
  }
}

void printClass(std::ostream& os,
                const cpp_class& e,
                const char* scope,
                const bool isPrivate,
                const bool enter) {
  if (enter) {
    os << "class " << e.name() << " ";
    if (e.is_final()) {
      os << "final ";
    }
    printBaseClasses(os, e.bases());
    os << " {\n";
    os << "public:\n";
    os << e.name() << "(std::shared_ptr<" << scope << "::" << e.name()
       << "> impl) \n";
    printBaseClassesConstructors(os, e.bases(), scope);
    if (!e.bases().empty()) {
      os << ", ";
    } else {
      os << ": ";
    }
    os << "_impl(std::move(impl))\n {}\n";
    os << "void setImpl(std::shared_ptr<" << scope << "::" << e.name()
       << "> impl)\n { _impl = std::move(impl); }\n";
  } else {
    if (!isPrivate) {
      os << "private:\n";
    }
    os << "std::shared_ptr<" << scope << "::" << e.name() << "> _impl;\n";
    os << "}; // class " << e.name() << "\n\n";
  }
}

}  // namespace tool
}  // namespace gunit
