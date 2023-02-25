/* Copyright(C) 2023 Hulevych */

#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/detail/assert.hpp>
#include <cppast/libclang_parser.hpp>
#include <cppast/visitor.hpp>

#include <cxxopts.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>

using namespace cppast;

namespace {
void print_help(const cxxopts::Options& options) {
  std::cout << options.help({"", "compilation"}) << '\n';
}

void print_error(const std::string& msg) {
  std::cerr << msg << '\n';
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

}  // namespace

constexpr const auto* GeneratedNamespaceName = "gunit_hook";

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

void printParams(
    std::ostream& os,
    const detail::iteratable_intrusive_list<cpp_function_parameter>& params,
    const bool includeTypes = false) {
  auto first = true;
  for (const auto& param : params) {
    if (!first) {
      os << ", ";
    } else {
      first = false;
    }
    if (includeTypes) {
      os << to_string(param.type()) << " ";
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
                       const FunctionType& e,
                       const char* scope,
                       const bool semicolon = false) {
  os << to_string(e.return_type()) << " ";
  if (scope != nullptr) {
    os << scope << "::";
  }
  os << e.name() << "(";
  const auto& params = e.parameters();
  printParams(os, params, true);
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

template <typename FunctionType>
void printFunctionBody(std::ostream& os,
                       const FunctionType& e,
                       const bool member,
                       const char* scope = nullptr) {
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
  printParams(os, params, false);
  os << ");\n";

  printFunctionNotify(os, member, !hasReturn, params.empty());
  printParams(os, params, false);
  os << ");\n";
  if (hasReturn) {
    os << "return result;\n";
  }
  os << CatchBlock << "}\n";
}

void printConstructorDecl(std::ostream& os,
                          const cpp_constructor& e,
                          const bool definition = false) {
  if (definition) {
    os << e.name() << "::";
  }
  os << e.name() << "(";
  const auto& params = e.parameters();
  printParams(os, params, true);
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
    os << base.name() << "(std::shared_ptr<" << scope << "::" << base.name() << ">())";
  }
}

void printConstructorBody(std::ostream& os,
                          const cpp_constructor& e,
                          const char* scope) {
  os << "{\n";
  os << "try {\n";
  os << "_impl = std::make_shared<" << scope << "::" << e.name() << ">(";

  const auto& params = e.parameters();
  printParams(os, params, false);
  os << ");\n";

  printConstructorNotify(os, params.empty());
  if(!params.empty()) {
      os << "(";
      printParams(os, params, false);
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
    os << e.name() << "(std::shared_ptr<" << scope << "::" << e.name() << "> impl) \n";
    printBaseClassesConstructors(os, e.bases(), scope);
    if(!e.bases().empty()) {
        os << ", ";
    } else {
        os << ": ";
    }
    os << "_impl(std::move(impl))\n {}\n";
    os << "void setImpl(std::shared_ptr<" << scope << "::" << e.name() << "> impl)\n { _impl = std::move(impl); }\n";
  } else {
    if (!isPrivate) {
      os << "private:\n";
    }
    os << "std::shared_ptr<" << scope << "::" << e.name() << "> _impl;\n";
    os << "}; // class " << e.name() << "\n\n";
  }
}

struct NamespacesStack final {
  void operator()(std::ostream& os) {
    if (!m_inside) {
      os << "namespace " << GeneratedNamespaceName << " {\n\n";
      m_inside = true;
    }
  }

  void push(std::ostream& os, const std::string& scope) {
    m_namespaces.push(scope);
    assert(!m_inside);
    printNamespace(os, m_namespaces.top(), true);
  }

  void pop(std::ostream& os) {
    if (m_inside) {
      os << "} // namespace " << GeneratedNamespaceName << std::endl;
      m_inside = false;
    }
    printNamespace(os, m_namespaces.top(), false);
    m_namespaces.pop();
  }

  const char* top() const {
    return !m_namespaces.empty() ? m_namespaces.top().c_str() : nullptr;
  }

 private:
  std::stack<std::string> m_namespaces;
  bool m_inside = false;
};

struct CodeGenerator {
  explicit CodeGenerator(std::ostream& out) : m_out(out) {}

  void handleAccess(const cpp_entity& e) {
    assert(e.kind() == cpp_entity_kind::access_specifier_t);
  }

  void handleClass(const cpp_class& e, const bool enter) {
      if(e.class_kind() != cpp_class_kind::class_t) { //TODO: rework
          return; }

    if (enter) {
      m_class = std::addressof(e);
    } else {
      m_class = nullptr;
    }
  }

  void handleNamespace(const cpp_entity& e, const bool enter) {
    if (enter) {
      m_namespaces.push(m_out, e.name());
    } else {
      m_namespaces.pop(m_out);
    }
  }

 protected:
  NamespacesStack m_namespaces;
  const cpp_class* m_class = nullptr;  // TODO: stack to support nested classes

  std::ostream& m_out;
};

struct HeaderCodeGenerator final : CodeGenerator {
  HeaderCodeGenerator(std::ostream& out) : CodeGenerator(out) {}
  ~HeaderCodeGenerator() = default;

  void onFileBegin(const cpp_file& file) {
    m_out << GeneratedFileHeader << "\n\n";
    m_out << "#pragma once\n\n";
    m_out << "#include <memory>\n\n";

    m_out << "#include \"" << file.name() << "\"\n\n";
  }

  void handleClass(const cpp_class& e, const bool enter) {
    if(e.class_kind() != cpp_class_kind::class_t) { //TODO: rework
        return; }

    CodeGenerator::handleClass(e, enter);

    if (enter) {
      m_namespaces(m_out);
    }
    printClass(m_out, e, m_namespaces.top(), m_access == "private", enter);
  }

  void handleConstructor(const cpp_constructor& e) {
    if(m_class == nullptr) { return; }

    assert(m_class->name() == e.name());
    printConstructorDecl(m_out, e, false);
  }

  void handleAccess(const cpp_entity& e) {
    if(m_class == nullptr) { return; }

    assert(e.kind() == cpp_entity_kind::access_specifier_t);
    if (m_access != e.name()) {
      m_access = e.name();
      m_out << e.name() << ":\n";
    }
  }

  void handleFreeFunction(const cpp_function& e) {
    m_namespaces(m_out);

    printFunctionDecl(m_out, e, nullptr, true);

    m_out << std::endl;
  }

  void handleMemberFunction(const cpp_member_function& e) {
    if(m_class == nullptr) { return; }

    m_namespaces(m_out);

    printFunctionDecl(m_out, e, nullptr, true);

    m_out << std::endl;
  }

 private:
  std::string m_access;
};

struct SourceCodeGenerator final : CodeGenerator {
  SourceCodeGenerator(const std::string& headerFile, std::ostream& out)
      : CodeGenerator(out), m_header(headerFile) {}
  ~SourceCodeGenerator() = default;

  void onFileBegin(const cpp_file&) {
    m_out << GeneratedFileHeader << "\n\n";

    m_out << "#include \"" << m_header << "\"\n\n";
    m_out << "#include <recorder/actions_observer.h>\n\n";
  }

  void handleConstructor(const cpp_constructor& e) {
      if(m_class == nullptr) { return; }

    assert(m_class->name() == e.name());
    m_namespaces(m_out);

    printConstructorDecl(m_out, e, true);
    printBaseClassesConstructors(m_out, m_class->bases(), m_namespaces.top());
    printConstructorBody(m_out, e, m_namespaces.top());

    m_out << std::endl;
  }

  void handleFreeFunction(const cpp_function& e) {
    m_namespaces(m_out);

    printFunctionDecl(m_out, e, nullptr, false);
    printFunctionBody(m_out, e, false, m_namespaces.top());

    m_out << std::endl;
  }

  void handleMemberFunction(const cpp_member_function& e) {
    if(m_class == nullptr) { return; }

    m_namespaces(m_out);

    printFunctionDecl(m_out, e, m_class->name().c_str(), false);
    printFunctionBody(m_out, e, true);

    m_out << std::endl;
  }

 private:
  const std::string m_header;
};

template <typename GeneratorType>
void process_file(GeneratorType& generator, const cpp_file& file) {
  generator.onFileBegin(file);
  visit(file, [&](const cpp_entity& e, visitor_info info) {
    const auto enter = info.event == visitor_info::container_entity_enter;
    switch (e.kind()) {
      case cpp_entity_kind::namespace_t:
        generator.handleNamespace(e, enter);
        break;
      case ::cpp_entity_kind::function_t:
        generator.handleFreeFunction(static_cast<const cpp_function&>(e));
        break;
      case ::cpp_entity_kind::class_t:
        generator.handleClass(static_cast<const cpp_class&>(e), enter);
        break;
      case ::cpp_entity_kind::constructor_t:
        generator.handleConstructor(static_cast<const cpp_constructor&>(e));
        break;
      case ::cpp_entity_kind::access_specifier_t:
        generator.handleAccess(e);
        break;
      case ::cpp_entity_kind::member_function_t:
        generator.handleMemberFunction(
            static_cast<const cpp_member_function&>(e));
        break;
      default:
        break;
    }
  });
}

std::unique_ptr<cpp_file> parse_file(const libclang_compile_config& config,
                                     const diagnostic_logger& logger,
                                     const std::string& filename,
                                     bool fatal_error) {
  // the entity index is used to resolve cross references in the AST
  // we don't need that, so it will not be needed afterwards
  cpp_entity_index idx;
  // the parser is used to parse the entity
  // there can be multiple parser implementations
  libclang_parser parser(type_safe::ref(logger));
  // parse the file
  auto file = parser.parse(idx, filename, config);
  if (fatal_error && parser.error())
    return nullptr;
  return file;
}

int main(int argc, char* argv[]) {
  try {
    cxxopts::Options option_list("gunit",
                                 "gunit generates recording wrappers.\n");
    // clang-format off
    option_list.add_options()
        ("h,help", "display this help and exit")
        ("version", "display version information and exit")
        ("v,verbose", "be verbose when parsing")
        ("fatal_errors", "abort program when a parser error occurs, instead of doing error correction")
        ("file", "the file that is being parsed (last positional argument)",
         cxxopts::value<std::string>())
        ("out_dir", "the directory which is used for output files generation",
         cxxopts::value<std::string>()->default_value("."));
        option_list.add_options("compilation")
        ("database_dir", "set the directory where a 'compile_commands.json' file is located containing build information",
            cxxopts::value<std::string>())
        ("database_file", "set the file name whose configuration will be used regardless of the current file name",
            cxxopts::value<std::string>())
        ("std", "set the C++ standard (c++98, c++03, c++11, c++14, c++1z (experimental), c++17, c++2a, c++20)",
         cxxopts::value<std::string>()->default_value(to_string(cpp_standard::cpp_latest)))
        ("I,include_directory", "add directory to include search path",
         cxxopts::value<std::vector<std::string>>())
        ("D,macro_definition", "define a macro on the command line",
         cxxopts::value<std::vector<std::string>>())
        ("U,macro_undefinition", "undefine a macro on the command line",
         cxxopts::value<std::vector<std::string>>())
        ("f,feature", "enable a custom feature (-fXX flag)",
         cxxopts::value<std::vector<std::string>>())
        ("gnu_extensions", "enable GNU extensions (equivalent to -std=gnu++XX)")
        ("msvc_extensions", "enable MSVC extensions (equivalent to -fms-extensions)")
        ("msvc_compatibility", "enable MSVC compatibility (equivalent to -fms-compatibility)")
        ("fast_preprocessing", "enable fast preprocessing, be careful, this breaks if you e.g. redefine macros in the same file!")
        ("remove_comments_in_macro", "whether or not comments generated by macro are kept, enable if you run into errors");
    // clang-format on
    option_list.parse_positional("file");

    auto options = option_list.parse(argc, argv);
    if (options.count("help"))
      print_help(option_list);
    else if (options.count("version")) {
      std::cout << "gunit version "
                << "1.0"
                << "\n";
    } else if (!options.count("file") ||
               options["file"].as<std::string>().empty()) {
      print_error("missing file argument");
      return 1;
    } else {
      // the compile config stores compilation flags
      libclang_compile_config config;
      if (options.count("database_dir")) {
        libclang_compilation_database database(
            options["database_dir"].as<std::string>());
        if (options.count("database_file"))
          config = libclang_compile_config(
              database, options["database_file"].as<std::string>());
        else
          config = libclang_compile_config(database,
                                           options["file"].as<std::string>());
      }

      if (options.count("verbose"))
        config.write_preprocessed(true);

      if (options.count("fast_preprocessing"))
        config.fast_preprocessing(true);

      if (options.count("remove_comments_in_macro"))
        config.remove_comments_in_macro(true);

      if (options.count("include_directory"))
        for (auto& include :
             options["include_directory"].as<std::vector<std::string>>())
          config.add_include_dir(include);
      if (options.count("macro_definition"))
        for (auto& macro :
             options["macro_definition"].as<std::vector<std::string>>()) {
          auto equal = macro.find('=');
          auto name = macro.substr(0, equal);
          if (equal == std::string::npos)
            config.define_macro(std::move(name), "");
          else {
            auto def = macro.substr(equal + 1u);
            config.define_macro(std::move(name), std::move(def));
          }
        }
      if (options.count("macro_undefinition"))
        for (auto& name :
             options["macro_undefinition"].as<std::vector<std::string>>())
          config.undefine_macro(name);
      if (options.count("feature"))
        for (auto& name : options["feature"].as<std::vector<std::string>>())
          config.enable_feature(name);

      // the compile_flags are generic flags
      compile_flags flags;
      if (options.count("gnu_extensions"))
        flags |= compile_flag::gnu_extensions;
      if (options.count("msvc_extensions"))
        flags |= compile_flag::ms_extensions;
      if (options.count("msvc_compatibility"))
        flags |= compile_flag::ms_compatibility;

      if (options["std"].as<std::string>() == "c++98")
        config.set_flags(cpp_standard::cpp_98, flags);
      else if (options["std"].as<std::string>() == "c++03")
        config.set_flags(cpp_standard::cpp_03, flags);
      else if (options["std"].as<std::string>() == "c++11")
        config.set_flags(cpp_standard::cpp_11, flags);
      else if (options["std"].as<std::string>() == "c++14")
        config.set_flags(cpp_standard::cpp_14, flags);
      else if (options["std"].as<std::string>() == "c++1z")
        config.set_flags(cpp_standard::cpp_1z, flags);
      else if (options["std"].as<std::string>() == "c++17")
        config.set_flags(cpp_standard::cpp_17, flags);
      else if (options["std"].as<std::string>() == "c++2a")
        config.set_flags(cpp_standard::cpp_2a, flags);
      else if (options["std"].as<std::string>() == "c++20")
        config.set_flags(cpp_standard::cpp_20, flags);
      else if (options["std"].as<std::string>() == "c++2b")
        config.set_flags(cpp_standard::cpp_2b, flags);
      else {
        print_error("invalid value '" + options["std"].as<std::string>() +
                    "' for std flag");
        return 1;
      }

      // the logger is used to print diagnostics
      stderr_diagnostic_logger logger;
      if (options.count("verbose"))
        logger.set_verbose(true);

      auto file = parse_file(config, logger, options["file"].as<std::string>(),
                             options.count("fatal_errors") == 1);
      if (!file)
        return 2;

      std::filesystem::path outFilePath;
      const auto fileName = std::filesystem::path(file->name()).stem();
      const auto outDir =
          std::filesystem::path(options["out_dir"].as<std::string>());
      std::filesystem::create_directories(outDir);
      if (std::filesystem::is_directory(outDir)) {
        outFilePath =
            std::filesystem::path(options["out_dir"].as<std::string>()) /
            fileName;
      }

      std::ofstream headerStream(outFilePath.string() + ".h");
      std::ofstream sourceStream(outFilePath.string() + ".cpp");

      HeaderCodeGenerator header(headerStream);
      SourceCodeGenerator source(outFilePath.string() + ".h", sourceStream);

      process_file(header, *file);
      process_file(source, *file);
    }
  } catch (const std::exception& ex) {
    print_error(std::string("[fatal parsing error] ") + ex.what());
    return 2;
  }
}
