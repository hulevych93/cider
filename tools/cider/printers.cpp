#include "printers.h"

#include "generator.h"
#include "utils.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_enum.hpp>
#include <cppast/cpp_expression.hpp>
#include <cppast/cpp_file.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>

#include <filesystem>

#include <cppast/detail/assert.hpp>

#include <assert.h>

using namespace cppast;

namespace cider {
namespace tool {

constexpr const auto* GeneratedFileHeader =
    R"(/* This file is auto-generated by cider tool */)";

constexpr const auto* CatchBlock = R"(} catch (const std::exception&) {
  // ex.what() notify
  throw;
} catch (...) {
  // NOTIFY some expection
  throw;
}
)";

namespace {

void printParamName(std::ostream& os,
                    const cpp_function_parameter& param,
                    unsigned int count = 0) {
  if (!param.name().empty()) {
    os << param.name();
  } else {
    os << "arg" << count;
  }
}

const cpp_type& remove_ref(const cpp_type& type) noexcept {
  if (type.kind() == cpp_type_kind::reference_t) {
    auto& ref = static_cast<const cpp_reference_type&>(type);
    return ref.referee();
  }

  return type;
}

void printAutoTypeDecl(std::ostream& os, const cpp_type& type) {
  if (type.kind() == cpp_type_kind::pointer_t) {
    const auto& pointer = static_cast<const cpp_pointer_type&>(type);
    printAutoTypeDecl(os, pointer.pointee());
    os << "*";
  } else if (type.kind() == cpp_type_kind::reference_t) {
    const auto& reference = static_cast<const cpp_reference_type&>(type);
    printAutoTypeDecl(os, reference.referee());
    os << "&";
  } else if (type.kind() == cpp_type_kind::cv_qualified_t) {
    const auto& cv = static_cast<const cpp_cv_qualified_type&>(type);
    if (cv.cv_qualifier() == cpp_cv::cpp_cv_const) {
      os << "const ";
    }
    printAutoTypeDecl(os, cv.type());
  } else {
    os << "auto";
  }
}

void printReturnStatement(std::ostream& os,
                          const cpp_type& type,
                          const std::optional<std::string> returnName,
                          const namespaces_stack& stack,
                          const MetadataStorage& metadata) {
  if (!returnName.has_value()) {
    return;
  }
  if (isUserData(type, stack.nativeScope(), metadata)) {
    auto value = to_string(type);
    replaceScope(stack.genScope(), value);

    if (isAggregate(type, stack.nativeScope(), metadata)) {
      os << "return ";
      if (!stack.genScope().empty()) {
        os << "cider::convert_out<";
        auto value = to_string(type);
        replaceScope(stack.genScope(), value);
        os << value;
        os << ">(";
      }
      os << returnName.value();
      if (!stack.genScope().empty()) {
        os << ")";
      }
      os << ";\n";
      return;
    }
  }
  os << "return " << returnName.value() << ";\n";
}

void printConstructorNotify(std::ostream& os, const bool hasNoArgs) {
  os << "CIDER_NOTIFY_CONSTRUCTOR";
  if (hasNoArgs) {
    os << "_NO_ARGS\n";
  }
}

void printFunctionNotify(std::ostream& os,
                         const bool member,
                         const std::optional<std::string> returnName,
                         const bool hasNoArgs) {
  os << "CIDER_NOTIFY_";
  if (member) {
    os << "METHOD";
  } else {
    os << "FREE_FUNCTION";
  }
  if (hasNoArgs) {
    os << "_NO_ARGS";
  }
  if (!returnName.has_value()) {
    os << "(std::nullopt";
  } else {
    os << "(" << returnName.value();
  }
  if (!hasNoArgs) {
    os << ", ";
  }
}

void printParamTypePure(std::ostream& os,
                        const MetadataStorage& metadata,
                        const namespaces_stack& stack,
                        const cpp_type& type) {
  if (isUserData(type, stack.nativeScope(),
                 metadata)) {  // check that pointer to user defined type!
    const auto& type_ = remove_ref(type);
    auto value = to_string(type_);
    replaceScope(stack.genScope(), value);
    os << value;
    return;
  }
  auto value = to_string(type);
  os << value;
}

void printParamType(std::ostream& os,
                    const MetadataStorage& metadata,
                    const namespaces_stack& stack,
                    const cpp_type& type) {
  auto value = to_string(type);
  if (isUserData(type, stack.nativeScope(),
                 metadata)) {  // check that pointer to user defined type!
    replaceScope(stack.genScope(), value);
  }
  os << value;
}

void printParamsDecl(
    std::ostream& os,
    const MetadataStorage& metadata,
    const namespaces_stack& stack,
    const detail::iteratable_intrusive_list<cpp_function_parameter>& params) {
  auto first = true;
  unsigned int count = 0U;
  for (const auto& param : params) {
    if (!first) {
      os << ", ";
    } else {
      first = false;
    }

    printParamType(os, metadata, stack, param.type());
    os << " ";

    printParamName(os, param, count);
    ++count;
  }
}

void printMethodCallOperator(std::ostream& os, const cpp_type& type) {
  if (type.kind() == cpp_type_kind::pointer_t) {
    os << "->";
  } else if (type.kind() == cpp_type_kind::reference_t) {
    os << ".";
  } else if (type.kind() == cpp_type_kind::cv_qualified_t) {
    const auto& cv = static_cast<const cpp_cv_qualified_type&>(type);
    printMethodCallOperator(os, cv.type());
  } else {
    os << ".";
  }
}

void printNewOperator(std::ostream& os, const cpp_type& type) {
  if (type.kind() == cpp_type_kind::pointer_t) {
    os << "new ";
  } else if (type.kind() == cpp_type_kind::reference_t) {
  } else if (type.kind() == cpp_type_kind::cv_qualified_t) {
    const auto& cv = static_cast<const cpp_cv_qualified_type&>(type);
    printNewOperator(os, cv.type());
  }
}

void printDereference(std::ostream& os, const cpp_type& type) {
  if (type.kind() == cpp_type_kind::reference_t) {
    os << "*";
  } else if (type.kind() == cpp_type_kind::cv_qualified_t) {
    const auto& cv = static_cast<const cpp_cv_qualified_type&>(type);
    printDereference(os, cv.type());
  }
}

void printParamsVal(
    std::ostream& os,
    const MetadataStorage& metadata,
    const detail::iteratable_intrusive_list<cpp_function_parameter>& params,
    const namespaces_stack& stack,
    const bool needDereference = true) {
  auto first = true;
  unsigned int count = 0U;
  for (const auto& param : params) {
    if (!first) {
      os << ", ";
    } else {
      first = false;
    }

    const auto& scope = stack.nativeScope();
    if (isUserData(param.type(), scope, metadata)) {
      if (isAggregate(param.type(), scope, metadata)) {
        os << "cider::convert_in<";
        auto value = to_string(param.type());
        replaceScope(stack.nativeScope(), value);
        os << value;
        os << ">(";
        printParamName(os, param, count);
        os << ")";
      } else {
        if (needDereference) {
          printDereference(os, param.type());
        }
        printParamName(os, param, count);
        printMethodCallOperator(os, param.type());
        os << "_impl.get()";
      }
    } else {
      printParamName(os, param, count);
    }

    ++count;
  }
}

void printBaseClasses(
    std::ostream& os,
    const MetadataStorage& metadata,
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

void printExpression(std::ostream& os, const cpp_expression& value) {
  if (value.kind() == cppast::cpp_expression_kind::literal_t) {
    const auto& literal =
        static_cast<const cppast::cpp_literal_expression&>(value);
    os << " = " << literal.value();
  } else if (value.kind() == cppast::cpp_expression_kind::unexposed_t) {
    const auto& expression =
        static_cast<const cppast::cpp_unexposed_expression&>(value);
    os << " = " << expression.expression().as_string();
  }
}

}  // namespace

void printNamespace(std::ostream& os,
                    const std::string& scope,
                    const bool enter) {
  if (enter) {
    os << "namespace " << scope << " {\n";
  } else {
    os << "} // namespace " << scope << "\n";
  }
}

template <typename FunctionType>
void printFunctionDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const FunctionType& e,
                       const namespaces_stack& stack,
                       const char* className,
                       const bool declaration) {
  if constexpr (std::is_same_v<cpp_member_function, FunctionType>) {
    const auto opType = isOperator(e);
    if (opType && ((*opType) == OperatorType::moveAssignment ||
                   (*opType) == OperatorType::copyAssignment)) {
      os << "void";
    } else {
      printParamTypePure(os, metadata, stack, e.return_type());
    }
  } else {
    printParamTypePure(os, metadata, stack, e.return_type());
  }

  os << " ";
  if (className != nullptr) {
    os << className << "::";
  }
  os << e.name() << "(";
  const auto& params = e.parameters();
  printParamsDecl(os, metadata, stack, params);
  os << ")";
  if constexpr (std::is_same_v<cpp_member_function, FunctionType>) {
    if (is_const(e.cv_qualifier())) {
      os << " const";
    }
  }
  if (declaration) {
    os << ";";
  }
  os << "\n";
}

void printFunctionDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cpp_function& e,
                       const namespaces_stack& stack,
                       const char* className,
                       const bool declaration) {
  printFunctionDecl<>(os, metadata, e, stack, className, declaration);
}

void printFunctionDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cpp_member_function& e,
                       const namespaces_stack& stack,
                       const char* className,
                       const bool declaration) {
  printFunctionDecl<>(os, metadata, e, stack, className, declaration);
}

template <typename FunctionType>
void printFunctionBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const char* obj,
                       const FunctionType& e,
                       const namespaces_stack& stack,
                       const bool member) {
  os << "{\n";
  os << "try {\n";
  std::string pureReturnTypeName;
  const auto hasImplemetation =
      isUserDefined(e.return_type(), stack.nativeScope(), pureReturnTypeName) &&
      isUserData(e.return_type(), stack.nativeScope(), metadata) &&
      !isAggregate(pureReturnTypeName, stack.nativeScope(), metadata);
  std::optional<std::string> returnName;
  std::optional<std::string> notificationName;  // TODO

  if constexpr (std::is_same_v<cpp_member_function, FunctionType>) {
    os << "auto* callee = cider::getCallee<" << stack.nativeScope() + "::" + obj
       << ">(this);\n";
  }

  if (hasImplemetation) {
    if (e.return_type().kind() == cpp_type_kind::pointer_t) {
      notificationName = "result->_impl.get()";
    } else {
      notificationName = "result._impl.get()";
    }
  }

  if (hasReturnValue(e)) {
    if (hasImplemetation) {
      printAutoTypeDecl(os, e.return_type());
      os << " impl = ";
    } else {
      printAutoTypeDecl(os, e.return_type());
      os << " result = ";
      returnName = "result";
      notificationName = returnName;
    }
  }
  const auto& scope = stack.nativeScope();
  const auto& genScope = stack.genScope();
  if constexpr (std::is_same_v<cpp_member_function, FunctionType>) {
    os << "callee->" << e.name() << "(";
  } else {
    os << scope << "::" << e.name() << "(";
  }
  const auto& params = e.parameters();
  printParamsVal(os, metadata, params, stack);
  os << ");\n";

  if (hasImplemetation) {
    if constexpr (std::is_same_v<cpp_member_function, FunctionType>) {
      os << "auto implPtr = cider::getImpl(impl, this);\n";
    } else {
      if (e.return_type().kind() == cpp_type_kind::pointer_t) {
        os << "auto implPtr = std::shared_ptr<" << pureReturnTypeName
           << ">(impl);\n";
        notificationName = "result->_impl.get()";
      } else {
        os << "auto implPtr = std::make_shared<" << pureReturnTypeName
           << ">(impl);\n";
        notificationName = "result._impl.get()";
      }
    }

    os << "auto result = ";
    returnName = "result";
    printNewOperator(os, e.return_type());
    auto value = pureReturnTypeName;
    replaceScope(genScope, value);
    os << value;
    os << "(implPtr);\n" << std::endl;
  }

  printFunctionNotify(os, member, notificationName, params.empty());

  printParamsVal(os, metadata, params, stack);
  os << ");\n";

  printReturnStatement(os, e.return_type(), returnName, stack, metadata);

  os << CatchBlock << "}\n";
}

void printOperatorBody(std::ostream& os,
                       const OperatorType& type,
                       const cppast::cpp_member_function& e) {
  os << "{\n";
  os << "try {\n";
  const auto& params = e.parameters();
  if (type == OperatorType::Equals) {
    os << "return (*_impl) == *";
    printParamName(os, *params.begin(), 0U);
    os << "._impl;\n";
  } else if (type == OperatorType::NotEquals) {
    os << "return (*_impl) != *";
    printParamName(os, *params.begin(), 0U);
    os << "._impl;\n";
  } else if (type == OperatorType::copyAssignment) {
    os << "CIDER_NOTIFY_ASSIGNMENT(";
    printParamName(os, *params.begin(), 0U);
    os << "._impl.get());\n";

    os << "(*_impl) = *";
    printParamName(os, *params.begin(), 0U);
    os << "._impl;\n";
  } else if (type == OperatorType::moveAssignment) {
    os << "CIDER_NOTIFY_ASSIGNMENT(";
    printParamName(os, *params.begin(), 0U);
    os << "._impl.get());\n";

    os << "(*_impl) = std::move(*";
    printParamName(os, *params.begin(), 0U);
    os << "._impl);\n";
  }
  os << CatchBlock;
  os << "}\n\n";
}

void printFunctionBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_function& e,
                       const namespaces_stack& stack) {
  printFunctionBody<>(os, metadata, nullptr, e, stack, false);
}

void printFunctionBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_class& cl,
                       const cppast::cpp_member_function& e,
                       const namespaces_stack& stack) {
  if (auto operatorType = isOperator(e)) {
    printOperatorBody(os, operatorType.value(), e);
  } else {
    printFunctionBody<>(os, metadata, cl.name().c_str(), e, stack, true);
  }
}

void printConstructorDecl(std::ostream& os,
                          const MetadataStorage& metadata,
                          const cpp_constructor& e,
                          const namespaces_stack& stack,
                          const bool definition) {
  if (definition) {
    os << e.name() << "::";
  }
  os << e.name() << "(";
  const auto& params = e.parameters();
  printParamsDecl(os, metadata, stack, params);
  os << ")";
  if (!definition) {
    os << ";";
  }
  os << "\n";
}

void printGeneratedMethods(std::ostream& os,
                           const MetadataStorage& metadata,
                           const cppast::cpp_class& e,
                           const namespaces_stack& stack,
                           const bool definition) {
  auto classMetaIt =
      metadata.classes.find(stack.nativeScope() + "::" + e.name());
  if (classMetaIt != metadata.classes.end()) {
    auto classMeta = classMetaIt->second;

    if (!classMeta.hasUserConstructors) {
      if (definition) {
        os << e.name() << "::";
      }
      os << e.name() << "()";
      if (definition) {
        os << " {\n";
        os << "try {\n";
        os << "_impl = std::make_shared<" << stack.nativeScope()
           << "::" << e.name() << ">();\n";
        os << "CIDER_NOTIFY_CONSTRUCTOR_NO_ARGS\n";
        os << CatchBlock;
        os << "}\n";
      } else {
        os << ";\n";
      }
    }

    if (!classMeta.hasCopyConstructor && !classMeta.hasMoveConstructor &&
        !classMeta.hasMoveAssignmentOperator) {
      if (definition) {
        os << e.name() << "::";
      }
      os << e.name() << "(const " << e.name() << "& other)";
      if (definition) {
        const auto& bases = e.bases();

        auto first = true;
        for (const auto& base : bases) {
          if (!first) {
            os << ", ";
          } else {
            os << ": ";
            first = false;
          }
          os << base.name() << "(other)";
        }

        os << " {\n";
        os << "try {\n";
        os << "_impl = std::make_shared<" << stack.nativeScope()
           << "::" << e.name() << ">(*other._impl.get());\n";
        os << "CIDER_NOTIFY_CONSTRUCTOR(other._impl.get())\n";
        os << CatchBlock;
        os << "}\n";
      } else {
        os << ";\n";
      }
    }

    if (!classMeta.hasMoveConstructor && !classMeta.hasCopyAssignmentOperator &&
        !classMeta.hasMoveAssignmentOperator) {
      os << "void ";
      if (definition) {
        os << e.name() << "::";
      }
      os << "operator=(const " << e.name() << "& other)";
      if (definition) {
        os << "{\n";
        os << "try {\n";
        os << "CIDER_NOTIFY_ASSIGNMENT(other._impl.get());\n";
        os << "(*_impl) = *other._impl;\n";
        os << CatchBlock;
        os << "}\n";
      } else {
        os << ";\n";
      }
    }

    if ((!classMeta.hasCopyAssignmentOperator &&
         !classMeta.hasMoveAssignmentOperator) &&
        !classMeta.hasMoveConstructor && !classMeta.hasCopyConstructor) {
      os << "void ";
      if (definition) {
        os << e.name() << "::";
      }
      os << "operator=(" << e.name() << "&& other)";
      if (definition) {
        os << "{\n";
        os << "try {\n";
        os << "CIDER_NOTIFY_ASSIGNMENT(other._impl.get());\n";
        os << "(*_impl) = std::move(*other._impl);\n";
        os << CatchBlock;
        os << "}\n";
      } else {
        os << ";\n";
      }
    }
  }
}

void printBaseClassesConstructors(
    std::ostream& os,
    const MetadataStorage& metadata,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases,
    const namespaces_stack& stack) {
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
    os << base.name() << "(std::shared_ptr<" << stack.nativeScope()
       << "::" << base.name() << ">())";
  }
}

void printBaseClassesSetImpl(
    std::ostream& os,
    const MetadataStorage& metadata,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases) {
  if (bases.empty()) {
    return;
  }

  auto first = true;
  for (const auto& base : bases) {
    if (!first) {
      os << ", ";
    } else {
      first = false;
    }
    os << base.name() << "::setImpl(_impl);\n";
  }
}

void printGeneralConstructorBody(
    std::ostream& os,
    const MetadataStorage& metadata,
    const cpp_constructor& e,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases,
    const namespaces_stack& stack) {
  os << "{\n";
  os << "try {\n";
  os << "_impl = std::make_shared<" << stack.nativeScope() << "::" << e.name()
     << ">(";

  const auto& params = e.parameters();
  printParamsVal(os, metadata, params, stack);
  os << ");\n";

  printConstructorNotify(os, params.empty());
  if (!params.empty()) {
    os << "(";
    printParamsVal(os, metadata, params, stack, false);
    os << ");\n";
  }

  printBaseClassesSetImpl(os, metadata, bases);

  os << CatchBlock << "}\n";
}

void printMoveConstructorBody(std::ostream& os, const cpp_constructor& e) {
  os << "{\n";
  os << "try {\n";
  os << "_impl = std::move(";

  const auto& params = e.parameters();
  printParamName(os, *params.begin(), 0U);
  os << "._impl);\n";

  os << CatchBlock << "}\n";
}

void printConstructorBody(
    std::ostream& os,
    const MetadataStorage& metadata,
    const cpp_constructor& e,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases,
    const namespaces_stack& stack) {
  if (isMoveContructor(e)) {
    printMoveConstructorBody(os, e);
  } else {
    printGeneralConstructorBody(os, metadata, e, bases, stack);
  }
}

void printClassDecl(std::ostream& os,
                    const MetadataStorage& metadata,
                    const cppast::cpp_class& e) {
  if (e.class_kind() == cpp_class_kind::class_t) {
    os << "class ";
  } else {
    os << "struct ";
  }

  os << e.name() << ";";
}

void printClassDef(std::ostream& os,
                   const MetadataStorage& metadata,
                   const cpp_class& e,
                   const namespaces_stack& stack,
                   const bool enter) {
  const auto aggregate = isAggregate(e.name(), stack.nativeScope(), metadata);
  const auto objName =
      e.class_kind() == cpp_class_kind::class_t ? "class" : "struct";
  const auto& scope = stack.nativeScope();
  if (enter) {
    os << objName << " " << e.name() << " ";
    if (e.is_final()) {
      os << "final ";
    }
    printBaseClasses(os, metadata, e.bases());
    os << " {\n";
    if (!aggregate) {
      os << "public:\n";
      os << e.name() << "(std::shared_ptr<" << scope << "::" << e.name()
         << "> impl) \n";
      printBaseClassesConstructors(os, metadata, e.bases(), stack);
      if (!e.bases().empty()) {
        os << ", ";
      } else {
        os << ": ";
      }
      os << "_impl(std::move(impl))\n {}\n";
      os << "void setImpl(std::shared_ptr<" << scope << "::" << e.name()
         << "> impl)\n { _impl = std::move(impl); }\n";
    }
  } else {
    if (!aggregate) {
      os << "std::shared_ptr<" << scope << "::" << e.name() << "> _impl;\n";
    }
    os << "};\n\n";
  }
}

void printEnum(std::ostream& os, const cppast::cpp_enum& e, const bool enter) {
  if (enter) {
    os << "enum ";
    if (e.is_scoped()) {
      os << "class ";
    }
    os << e.name();
    if (e.has_explicit_type()) {
      os << " : ";
      os << to_string(e.underlying_type());
    }
    os << " {\n";
  } else {
    os << "};// enum " << e.name() << "\n\n";
  }
}

void printEnumValue(std::ostream& os, const cppast::cpp_enum_value& e) {
  os << e.name();
  if (const auto& value = e.value()) {
    printExpression(os, value.value());
  }
  os << ", ";
}

void printVariableDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_member_variable& e,
                       const namespaces_stack& stack) {
  printParamType(os, metadata, stack, e.type());
  os << " ";
  os << e.name();
  std::cerr << e.name() << " ";
  if (const auto& defaultValue = e.default_value()) {
    printExpression(os, defaultValue.value());
  }
  os << ";\n";
}

void printIncludes(std::ostream& os,
                   const MetadataStorage& metadata,
                   const cppast::cpp_file& file) {
  os << "#include \"" << file.name() << "\"\n\n";

  const auto& fileMetadata = metadata.files.at(file.name());
  const auto& classesMetadata = metadata.classes;

  const auto& imports = fileMetadata.imports;
  for (const auto& import : imports) {
    auto it = classesMetadata.find(import);
    if (it != classesMetadata.end()) {
      const auto& classMetadata = it->second;
      const auto fileName =
          std::filesystem::path(classMetadata.file).stem().string();
      os << "#include \"" << fileName << ".h\"\n";
    }
  }
  os << std::endl;
}

void printHeader(std::ostream& os,
                 const MetadataStorage& metadata,
                 const std::string& genScope,
                 const cppast::cpp_file& file) {
  header_generator header(os, std::move(genScope), metadata);

  os << GeneratedFileHeader << "\n\n";
  os << "#pragma once\n\n";
  os << "#include <memory>\n\n";

  printIncludes(os, metadata, file);

  handleFile(header, file);
}

void printSource(std::ostream& os,
                 const MetadataStorage& metadata,
                 const std::string& genScope,
                 const std::string& outputFile,
                 const cppast::cpp_file& file) {
  source_generator source(os, std::move(genScope), metadata);

  os << GeneratedFileHeader << "\n\n";
  os << "#include \"" << outputFile + ".h"
     << "\"\n\n";
  os << "#include <recorder/actions_observer.h>\n\n";
  os << "#include <utils/convert_utils.h>\n\n";
  os << "#include <utils/call_utils.h>\n\n";

  handleFile(source, file);
}

}  // namespace tool
}  // namespace cider
