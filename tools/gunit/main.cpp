/* Copyright(C) 2023 Hulevych */

#include <cppast/libclang_parser.hpp>

#include <cxxopts.hpp>

#include <filesystem>

#include "ast_handler.h"
#include "generator.h"
#include "namespaces_stack.h"
#include "options.h"
#include "printers.h"

using namespace cppast;
using namespace gunit::tool;

namespace {

void print_error(const std::string& msg) {
  std::cerr << msg << '\n';
}

void print_help(const cxxopts::Options& options) {
  std::cout << options.help({"", "compilation"}) << '\n';
}

std::unique_ptr<cpp_file> parse_file(const libclang_compile_config& config,
                                     const diagnostic_logger& logger,
                                     const std::string& filename,
                                     bool fatal_error) {
  // the parser is used to parse the entity
  // there can be multiple parser implementations
  libclang_parser parser(type_safe::ref(logger));
  // parse the file
  cpp_entity_index idx;
  auto file = parser.parse(idx, filename, config);
  if (fatal_error && parser.error())
    return nullptr;
  return file;
}

}  // namespace

int main(int argc, char* argv[]) {
  try {
    auto option_list = getOptions();
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
      const libclang_compile_config config = buildConfig(options);

      // the logger is used to print diagnostics
      stderr_diagnostic_logger logger;
      if (options.count("verbose"))
        logger.set_verbose(true);

      auto file = parse_file(config, logger, options["file"].as<std::string>(),
                             options.count("fatal_errors") == 1);
      if (!file)
        return 2;

      const auto outFilePath =
          getOutputFilePathWithoutExtension(file->name(), options);

      MetadataStorage metadata;
      metadata_collector collector(metadata);
      process_file(collector, *file);

      printHeader(outFilePath, metadata, *file);
      printSource(outFilePath, metadata, *file);
    }
  } catch (const std::exception& ex) {
    print_error(std::string("[fatal parsing error] ") + ex.what());
    return 2;
  }
}
