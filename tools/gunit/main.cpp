/* Copyright(C) 2023 Hulevych */

#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/detail/assert.hpp>
#include <cppast/libclang_parser.hpp>
#include <cppast/visitor.hpp>

#include <cxxopts.hpp>

#include <assert.h>
#include <filesystem>
#include <iostream>

// print help options
void print_help(const cxxopts::Options& options) {
  std::cout << options.help({"", "compilation"}) << '\n';
}

// print error message
void print_error(const std::string& msg) {
  std::cerr << msg << '\n';
}

bool is_entity_processed(const cppast::cpp_entity& e) {
  return (!cppast::is_templated(e) &&
          (e.kind() == cppast::cpp_entity_kind::constructor_t ||
           e.kind() == cppast::cpp_entity_kind::destructor_t ||
           e.kind() == cppast::cpp_entity_kind::function_t ||
           e.kind() == cppast::cpp_entity_kind::member_function_t)) ||
         e.kind() == cppast::cpp_entity_kind::namespace_t ||
         e.kind() == cppast::cpp_entity_kind::include_directive_t;
}

bool hasReturnValue(const cppast::cpp_function& e) {
  const auto& type = e.return_type();
  if (type.kind() == cppast::cpp_type_kind::builtin_t) {
    const auto& buildInType =
        static_cast<const cppast::cpp_builtin_type&>(type);
    return buildInType.builtin_type_kind() !=
           cppast::cpp_builtin_type_kind::cpp_void;
  }
  return true;
}

constexpr const auto* GeneratedNamespaceName = "gunit_hook";

constexpr const auto* CatchBlock = R"(} catch (const std::exception&) {
  // ex.what() notify
  throw;
} catch (...) {
  // NOTIFY some expection
  throw;
})";

struct HeaderCodeGenerator final {
  HeaderCodeGenerator(const cppast::cpp_file& file, std::ostream& out)
      : m_out(out) {
    m_out << "// File is auto-generated by gunit tool.\n\n";
    m_out << "#pragma once\n\n";

    std::filesystem::path cppFile(file.name());
    m_fileName = cppFile.stem();
  }

  ~HeaderCodeGenerator() { m_out << "---------------------" << std::endl; }

  void handleInclude(const cppast::cpp_entity& e) {
    assert(e.kind() == cppast::cpp_entity_kind::include_directive_t);
    if (e.name().find(m_fileName) != std::string::npos) {
      m_out << "include \"" << e.name() << "\"\n\n";
    }
  }

  void handleNamespace(const cppast::cpp_entity& e, const bool enter) {
    assert(e.kind() == cppast::cpp_entity_kind::namespace_t);
    if (enter) {
      m_out << "namespace " << e.name() << " {\n";
      m_refNamespace = e.name();
    } else {
      generateNamespace(false);
      m_out << "} // namespace " << e.name() << "\n";
    }
  }

  void handleFreeFunction(const cppast::cpp_function& e) {
    generateNamespace(true);

    m_out << cppast::to_string(e.return_type());
    m_out << " " << e.name() << "(";

    const auto& parameters = e.parameters();
    auto first = true;
    for (const auto& param : parameters) {
      if (!first) {
        m_out << ", ";
      } else {
        first = false;
      }
      m_out << cppast::to_string(param.type());

      if (!param.name().empty()) {
        m_out << " " << param.name();
      }
    }

    m_out << ")\n";
  }

 private:
  void generateNamespace(const bool enter) {
    if (enter && !m_insideNamespace) {
      m_out << "namespace " << GeneratedNamespaceName << " {\n\n";
      m_insideNamespace = true;
    }
    if (!enter && m_insideNamespace) {
      m_out << "\n\n} // namespace " << GeneratedNamespaceName << std::endl;
      m_insideNamespace = false;
    }
  }

 private:
  bool m_insideNamespace = false;
  std::string m_refNamespace;

  std::string m_fileName;
  std::ostream& m_out;
};

struct SourceCodeGenerator final {
  SourceCodeGenerator(const std::string& headerFile, std::ostream& out)
      : m_out(out) {
    m_out << "// File is auto-generated by gunit tool.\n\n";
    m_out << "#include " << headerFile << "\n\n";
    m_out << "#include \"recorder/actions_observer.h\"\n\n";
  }

  ~SourceCodeGenerator() { m_out << "---------------------" << std::endl; }

  void handleNamespace(const cppast::cpp_entity& e, const bool enter) {
    assert(e.kind() == cppast::cpp_entity_kind::namespace_t);
    if (enter) {
      m_out << "namespace " << e.name() << " {\n";
      m_refNamespace = e.name();
    } else {
      generateNamespace(false);
      m_out << "} // namespace " << e.name() << "\n";
    }
  }

  void handleFreeFunction(const cppast::cpp_function& e) {
    generateNamespace(true);

    m_out << cppast::to_string(e.return_type());
    m_out << " " << e.name() << "(";

    const auto& parameters = e.parameters();
    auto first = true;
    for (const auto& param : parameters) {
      if (!first) {
        m_out << ", ";
      } else {
        first = false;
      }
      m_out << cppast::to_string(param.type());
      // assert(!param.name().empty());
      if (!param.name().empty()) {
        m_out << " " << param.name();
      }
    }

    m_out << ")\n";

    m_out << "{\n";
    m_out << "try {\n";

    const auto hasReturn = hasReturnValue(e);

    if (hasReturn) {
      m_out << "auto result = ";
    }
    m_out << m_refNamespace << "::" << e.name() << "(";

    first = true;
    for (const auto& param : parameters) {
      if (!first) {
        m_out << ", ";
      } else {
        first = false;
      }

      if (!param.name().empty()) {
        m_out << param.name();
      }
    }

    m_out << ");\n";

    if (hasReturn) {
      m_out << "GUNIT_NOTIFY_FREE_FUNCTION(";
    } else {
      m_out << "GUNIT_NOTIFY_FREE_FUNCTION_NO_RETURN(";
    }

    first = true;
    for (const auto& param : parameters) {
      if (!first) {
        m_out << ", ";
      } else {
        first = false;
      }

      if (!param.name().empty()) {
        m_out << param.name();
      }
    }

    m_out << ");\n";

    if (hasReturn) {
      m_out << "return result;\n";
    }

    m_out << CatchBlock << std::endl;
  }

 private:
  void generateNamespace(const bool enter) {
    if (enter && !m_insideNamespace) {
      m_out << "namespace " << GeneratedNamespaceName << " {\n\n";
      m_insideNamespace = true;
    }
    if (!enter && m_insideNamespace) {
      m_out << "\n\n} // namespace " << GeneratedNamespaceName << std::endl;
      m_insideNamespace = false;
    }
  }

 private:
  bool m_insideNamespace = false;
  std::string m_refNamespace;

  std::ostream& m_out;
};

void process_file(const cppast::cpp_file& file) {
  std::stringstream headerFile;
  std::stringstream sourceFile;

  HeaderCodeGenerator header(file, headerFile);
  SourceCodeGenerator source("qqq", std::cout);
  cppast::visit(
      file, [](const cppast::cpp_entity& e) { return is_entity_processed(e); },
      [&](const cppast::cpp_entity& e, cppast::visitor_info info) {
        switch (e.kind()) {
          case cppast::cpp_entity_kind::namespace_t:
            header.handleNamespace(
                e, info.event == cppast::visitor_info::container_entity_enter);
            source.handleNamespace(
                e, info.event == cppast::visitor_info::container_entity_enter);
            break;
          case ::cppast::cpp_entity_kind::include_directive_t:
            header.handleInclude(e);
            break;
          case ::cppast::cpp_entity_kind::function_t:
            header.handleFreeFunction(
                static_cast<const cppast::cpp_function&>(e));
            source.handleFreeFunction(
                static_cast<const cppast::cpp_function&>(e));
            break;
          default:
            break;
        }
      });
}

// parse a file
std::unique_ptr<cppast::cpp_file> parse_file(
    const cppast::libclang_compile_config& config,
    const cppast::diagnostic_logger& logger,
    const std::string& filename,
    bool fatal_error) {
  // the entity index is used to resolve cross references in the AST
  // we don't need that, so it will not be needed afterwards
  cppast::cpp_entity_index idx;
  // the parser is used to parse the entity
  // there can be multiple parser implementations
  cppast::libclang_parser parser(type_safe::ref(logger));
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
             cxxopts::value<std::string>());
        option_list.add_options("compilation")
            ("database_dir", "set the directory where a 'compile_commands.json' file is located containing build information",
            cxxopts::value<std::string>())
            ("database_file", "set the file name whose configuration will be used regardless of the current file name",
            cxxopts::value<std::string>())
            ("std", "set the C++ standard (c++98, c++03, c++11, c++14, c++1z (experimental), c++17, c++2a, c++20)",
             cxxopts::value<std::string>()->default_value(cppast::to_string(cppast::cpp_standard::cpp_latest)))
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
      cppast::libclang_compile_config config;
      if (options.count("database_dir")) {
        cppast::libclang_compilation_database database(
            options["database_dir"].as<std::string>());
        if (options.count("database_file"))
          config = cppast::libclang_compile_config(
              database, options["database_file"].as<std::string>());
        else
          config = cppast::libclang_compile_config(
              database, options["file"].as<std::string>());
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
      cppast::compile_flags flags;
      if (options.count("gnu_extensions"))
        flags |= cppast::compile_flag::gnu_extensions;
      if (options.count("msvc_extensions"))
        flags |= cppast::compile_flag::ms_extensions;
      if (options.count("msvc_compatibility"))
        flags |= cppast::compile_flag::ms_compatibility;

      if (options["std"].as<std::string>() == "c++98")
        config.set_flags(cppast::cpp_standard::cpp_98, flags);
      else if (options["std"].as<std::string>() == "c++03")
        config.set_flags(cppast::cpp_standard::cpp_03, flags);
      else if (options["std"].as<std::string>() == "c++11")
        config.set_flags(cppast::cpp_standard::cpp_11, flags);
      else if (options["std"].as<std::string>() == "c++14")
        config.set_flags(cppast::cpp_standard::cpp_14, flags);
      else if (options["std"].as<std::string>() == "c++1z")
        config.set_flags(cppast::cpp_standard::cpp_1z, flags);
      else if (options["std"].as<std::string>() == "c++17")
        config.set_flags(cppast::cpp_standard::cpp_17, flags);
      else if (options["std"].as<std::string>() == "c++2a")
        config.set_flags(cppast::cpp_standard::cpp_2a, flags);
      else if (options["std"].as<std::string>() == "c++20")
        config.set_flags(cppast::cpp_standard::cpp_20, flags);
      else if (options["std"].as<std::string>() == "c++2b")
        config.set_flags(cppast::cpp_standard::cpp_2b, flags);
      else {
        print_error("invalid value '" + options["std"].as<std::string>() +
                    "' for std flag");
        return 1;
      }

      // the logger is used to print diagnostics
      cppast::stderr_diagnostic_logger logger;
      if (options.count("verbose"))
        logger.set_verbose(true);

      auto file = parse_file(config, logger, options["file"].as<std::string>(),
                             options.count("fatal_errors") == 1);
      if (!file)
        return 2;
      process_file(*file);
    }
  } catch (const cppast::libclang_error& ex) {
    print_error(std::string("[fatal parsing error] ") + ex.what());
    return 2;
  }
}
