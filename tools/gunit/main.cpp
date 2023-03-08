/* Copyright(C) 2023 Hulevych */

#include <cppast/libclang_parser.hpp>

#include <cxxopts.hpp>

#include <filesystem>
#include <fstream>

#include "ast_handler.h"
#include "generator.h"
#include "namespaces_stack.h"
#include "options.h"
#include "printers.h"
#include "utils.h"

using namespace cppast;
using namespace gunit::tool;

namespace {

void print_error(const std::string& msg) {
  std::cerr << msg << '\n';
}

void print_help(const cxxopts::Options& options) {
  std::cout << options.help({"", "compilation"}) << '\n';
}

template <typename Functor>
auto parse_files(const libclang_compile_config& config,
                 const diagnostic_logger& logger,
                 const std::vector<std::string>& files,
                 const bool fatal_error,
                 Functor&& func) {
  cpp_entity_index idx;
  cppast::simple_file_parser<cppast::libclang_parser> parser(
      type_safe::ref(idx), type_safe::ref(logger));
  for (const auto& file : files) {
    parser.parse(file, config);
    if (fatal_error && parser.error()) {
      return;
    }
  }

  const auto& result = parser.files();
  for (const auto& file : result) {
    func(file);
  }
}

auto getFiles(const cxxopts::ParseResult& options) {
  std::vector<std::string> files;
  if (options.count("file") && !options["file"].as<std::string>().empty()) {
    files.emplace_back(options["file"].as<std::string>());
  }
  if (options.count("files") && !options["files"].as<std::string>().empty()) {
    const auto& filesOption = options["files"].as<std::string>();

    std::string file;
    std::istringstream iss{filesOption};
    while (std::getline(iss, file, ':')) {
      files.emplace_back(std::move(file));
    }
  }
  return files;
}

}  // namespace

int main(int argc, char* argv[]) {
  try {
    auto option_list = getOptions();
    option_list.parse_positional("file");

    auto options = option_list.parse(argc, argv);
    if (options.count("help")) {
      print_help(option_list);
      return 0;
    }
    if (options.count("version")) {
      std::cout << "gunit version "
                << "1.0"
                << "\n";
      return 0;
    }

    const auto& files = getFiles(options);
    if (files.empty()) {
      print_error("missing file(s) argument");
      return 1;
    } else {
      // the compile config stores compilation flags
      const libclang_compile_config config = buildConfig(options);

      // the logger is used to print diagnostics
      stderr_diagnostic_logger logger;
      if (options.count("verbose"))
        logger.set_verbose(true);

      cpp_entity_index idx;
      cppast::simple_file_parser<cppast::libclang_parser> parser(
          type_safe::ref(idx), type_safe::ref(logger));
      for (const auto& file : files) {
        parser.parse(file, config);
        if (options.count("fatal_errors") == 1 && parser.error()) {
          return 1;
        }
      }

      const auto& result = parser.files();
      const auto& metadata = collectMetadata(result);

      for (const auto& file : result) {
        const auto outFilePath =
            getOutputFilePathWithoutExtension(file.name(), options);

        const auto genScope = options["namespace"].as<std::string>();

        std::ofstream headerStream(outFilePath + ".h");
        printHeader(headerStream, metadata, genScope, file);

        std::ofstream sourceStream(outFilePath + ".cpp");
        printSource(sourceStream, metadata, genScope, outFilePath, file);
      }
    }
  } catch (const std::exception& ex) {
    print_error(std::string("[fatal parsing error] ") + ex.what());
    return 2;
  }
}
