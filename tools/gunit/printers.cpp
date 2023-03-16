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

namespace gunit {
namespace tool {

constexpr const auto* GeneratedFileHeader =
    R"(/* This file is auto-generated by gunit tool */)";

constexpr const auto* CatchBlock = R"(} catch (const std::exception&) {
  // ex.what() notify
  throw;
} catch (...) {
  // NOTIFY some expection
  throw;
}
)";

namespace {

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
                          const char* genScope,
                          const MetadataStorage& metadata) {
  if (!returnName.has_value()) {
    return;
  }
  if (isUserData(type, metadata)) {
    auto value = to_string(type);
    replaceScope(genScope, value);

    if (isAggregate(type, metadata)) {
      os << "return *reinterpret_cast<" + value + "*>(&" << returnName.value()
         << ");\n";
      return;
    }
  }

  os << "return " << returnName.value() << ";\n";
}

void printConstructorNotify(std::ostream& os, const bool hasNoArgs) {
  os << "GUNIT_NOTIFY_CONSTRUCTOR";
  if (hasNoArgs) {
    os << "_NO_ARGS\n";
  }
}

void printFunctionNotify(std::ostream& os,
                         const bool member,
                         const std::optional<std::string> returnName,
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
  if (!returnName.has_value()) {
    os << "(std::nullopt";
  } else {
    os << "(" << returnName.value();
  }
  if (!hasNoArgs) {
    os << ", ";
  }
}

void printParamType(std::ostream& os,
                    const MetadataStorage& metadata,
                    const std::string& genScope,
                    const cpp_type& type) {
  auto value = to_string(type);
  if (isUserData(type, metadata)) {  // check that pointer to user defined type!
    replaceScope(genScope, value);
  }
  os << value;
}

void printParamsDecl(
    std::ostream& os,
    const MetadataStorage& metadata,
    const std::string& genScope,
    const detail::iteratable_intrusive_list<cpp_function_parameter>& params) {
  auto first = true;
  for (const auto& param : params) {
    if (!first) {
      os << ", ";
    } else {
      first = false;
    }

    printParamType(os, metadata, genScope, param.type());
    os << " ";

    assert(!param.name().empty());
    if (!param.name().empty()) {
      os << param.name();
    }
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
    const bool needDereference = true) {
  auto first = true;
  for (const auto& param : params) {
    if (!first) {
      os << ", ";
    } else {
      first = false;
    }

    assert(!param.name().empty());
    if (!param.name().empty()) {
      if (isUserData(param.type(), metadata)) {
        if (isAggregate(param.type(), metadata)) {
          os << "(" + to_string(param.type()) + ")(";
          os << param.name() << ")";
        } else {
          if (needDereference) {
            printDereference(os, param.type());
          }
          os << param.name();
          printMethodCallOperator(os, param.type());
          os << "_impl.get()";
        }
      } else {
        os << param.name();
      }
    }
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
                       const char* genScope,
                       const char* scope,
                       const bool semicolon) {
  printParamType(os, metadata, genScope, e.return_type());
  os << " ";
  if (scope != nullptr) {
    os << scope << "::";
  }
  os << e.name() << "(";
  const auto& params = e.parameters();
  printParamsDecl(os, metadata, genScope, params);
  os << ")";
  if constexpr (std::is_same_v<cpp_member_function, FunctionType>) {
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
                       const MetadataStorage& metadata,
                       const cpp_function& e,
                       const char* genScope,
                       const char* scope,
                       const bool semicolon) {
  printFunctionDecl<>(os, metadata, e, genScope, scope, semicolon);
}

void printFunctionDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cpp_member_function& e,
                       const char* genScope,
                       const char* scope,
                       const bool semicolon) {
  printFunctionDecl<>(os, metadata, e, genScope, scope, semicolon);
}

template <typename FunctionType>
void printFunctionBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const FunctionType& e,
                       const bool member,
                       const char* genScope,
                       const char* scope) {
  os << "{\n";
  os << "try {\n";
  std::string pureReturnTypeName;
  const auto hasImplemetation =
      hasImpl(e.return_type(), metadata, pureReturnTypeName);
  std::optional<std::string> returnName;
  std::optional<std::string> notificationName;  // TODO
  if (hasReturnValue(e)) {
    if (hasImplemetation) {
      os << "auto impl = ";
    } else {
      printAutoTypeDecl(os, e.return_type());
      os << " result = ";
      returnName = "result";
      notificationName = returnName;
    }
  }
  if constexpr (std::is_same_v<cpp_member_function, FunctionType>) {
    os << "_impl->" << e.name() << "(";
  } else {
    os << scope << "::" << e.name() << "(";
  }
  const auto& params = e.parameters();
  printParamsVal(os, metadata, params);
  os << ");\n";

  if (hasImplemetation) {
    os << "auto result = ";
    returnName = "result";
    printNewOperator(os, e.return_type());
    auto value = pureReturnTypeName;
    replaceScope(genScope, value);
    os << value;
    if (e.return_type().kind() == cpp_type_kind::pointer_t) {
      os << "(std::shared_ptr<" << pureReturnTypeName << ">(impl));\n";
      notificationName = "result->_impl.get()";
    } else {
      os << "(std::make_shared<" << pureReturnTypeName << ">(impl));\n";
      notificationName = "result._impl.get()";
    }
  }

  printFunctionNotify(os, member, notificationName, params.empty());

  printParamsVal(os, metadata, params);
  os << ");\n";

  printReturnStatement(os, e.return_type(), returnName, genScope, metadata);

  os << CatchBlock << "}\n";
}

void printOperatorBody(std::ostream& os,
                       const OperatorType& type,
                       const cppast::cpp_member_function& e) {
  os << "{\n";
  os << "try {\n";
  auto returnThis = false;  // TODO
  const auto& params = e.parameters();
  if (type == OperatorType::Equals) {
    os << "return (*_impl) == *";
    os << params.begin()->name();
    os << "._impl;\n";
  } else if (type == OperatorType::NotEquals) {
    os << "return (*_impl) != *";
    os << params.begin()->name();
    os << "._impl;\n";
  } else if (type == OperatorType::copyAssignment) {
    os << "GUNIT_NOTIFY_ASSIGNMENT(";
    os << params.begin()->name();
    os << "._impl.get());\n";

    os << "(*_impl) = *";
    os << params.begin()->name();
    os << "._impl;\n";
    returnThis = true;
  } else if (type == OperatorType::moveAssignment) {
    os << "GUNIT_NOTIFY_ASSIGNMENT(";
    os << params.begin()->name();
    os << "._impl.get());\n";

    os << "(*_impl) = std::move(*";
    os << params.begin()->name();
    os << "._impl);\n";
    returnThis = true;
  }
  os << CatchBlock;
  if (returnThis) {
    os << "return *this;\n";
  }
  os << "}\n";
}

void printFunctionBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_function& e,
                       const char* genScope,
                       const char* scope) {
  printFunctionBody<>(os, metadata, e, false, genScope, scope);
}

void printFunctionBody(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_member_function& e,
                       const char* genScope,
                       const char* scope) {
  if (auto operatorType = isOperator(e)) {
    printOperatorBody(os, operatorType.value(), e);
  } else {
    printFunctionBody<>(os, metadata, e, true, genScope, scope);
  }
}

void printConstructorDecl(std::ostream& os,
                          const MetadataStorage& metadata,
                          const cpp_constructor& e,
                          const char* genScope,
                          const bool definition) {
  if (definition) {
    os << e.name() << "::";
  }
  os << e.name() << "(";
  const auto& params = e.parameters();
  printParamsDecl(os, metadata, genScope, params);
  os << ")";
  if (!definition) {
    os << ";";
  }
  os << "\n";
}

void printGeneratedMethods(std::ostream& os,
                           const MetadataStorage& metadata,
                           const cppast::cpp_class& e,
                           const char* scope,
                           const bool definition) {
  auto classMetaIt =
      metadata.classes.find(std::string{scope} + "::" + e.name());
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
        os << "_impl = std::make_shared<" << scope << "::" << e.name()
           << ">();\n";
        os << "GUNIT_NOTIFY_CONSTRUCTOR_NO_ARGS\n";
        os << CatchBlock;
        os << "}\n";
      } else {
        os << ";\n";
      }
    }

    if (!classMeta.hasCopyConstructor) {
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
        os << "_impl = std::make_shared<" << scope << "::" << e.name()
           << ">(*other._impl.get());\n";
        os << "GUNIT_NOTIFY_CONSTRUCTOR(other._impl.get())\n";
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
    const char* genScope,
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
    const char* scope) {
  os << "{\n";
  os << "try {\n";
  os << "_impl = std::make_shared<" << scope << "::" << e.name() << ">(";

  const auto& params = e.parameters();
  printParamsVal(os, metadata, params);
  os << ");\n";

  printConstructorNotify(os, params.empty());
  if (!params.empty()) {
    os << "(";
    printParamsVal(os, metadata, params, false);
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
  os << params.begin()->name();
  os << "._impl);\n";

  os << CatchBlock << "}\n";
}

void printConstructorBody(
    std::ostream& os,
    const MetadataStorage& metadata,
    const cpp_constructor& e,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases,
    const char* scope) {
  if (isMoveContructor(e)) {
    printMoveConstructorBody(os, e);
  } else {
    printGeneralConstructorBody(os, metadata, e, bases, scope);
  }
}

void printClass(std::ostream& os,
                const MetadataStorage& metadata,
                const cpp_class& e,
                const char* genScope,
                const char* scope,
                const bool enter) {
  if (enter) {
    os << "class " << e.name() << " ";
    if (e.is_final()) {
      os << "final ";
    }
    printBaseClasses(os, metadata, e.bases());
    os << " {\n";
    os << "public:\n";
    os << e.name() << "(std::shared_ptr<" << scope << "::" << e.name()
       << "> impl) \n";
    printBaseClassesConstructors(os, metadata, e.bases(), genScope, scope);
    if (!e.bases().empty()) {
      os << ", ";
    } else {
      os << ": ";
    }
    os << "_impl(std::move(impl))\n {}\n";
    os << "void setImpl(std::shared_ptr<" << scope << "::" << e.name()
       << "> impl)\n { _impl = std::move(impl); }\n";
  } else {
    os << "std::shared_ptr<" << scope << "::" << e.name() << "> _impl;\n";
    os << "}; // class " << e.name() << "\n\n";
  }
}

void printStruct(std::ostream& os,
                 const MetadataStorage& metadata,
                 const cppast::cpp_class& e,
                 const bool enter) {
  if (enter) {
    os << "struct " << e.name() << " ";
    if (e.is_final()) {
      os << "final ";
    }
    printBaseClasses(os, metadata, e.bases());
    os << " {\n";
  } else {
    os << "}; // struct " << e.name() << "\n\n";
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
    os << ", ";
  }
}

void printVariableDecl(std::ostream& os,
                       const MetadataStorage& metadata,
                       const cppast::cpp_member_variable& e,
                       const char* genScope) {
  printParamType(os, metadata, genScope, e.type());
  os << " ";
  os << e.name();
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

  handleFile(source, file);
}

}  // namespace tool
}  // namespace gunit
