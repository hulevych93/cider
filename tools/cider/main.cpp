// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include <cppast/libclang_parser.hpp>

#include <cxxopts.hpp>

#include <filesystem>
#include <fstream>
#include <thread>

#include <cppast/cpp_entity.hpp>

#include "ast_handler.h"
#include "generator.h"
#include "lua.h"
#include "namespaces_stack.h"
#include "options.h"
#include "printers.h"
#include "swig.h"
#include "utils.h"

using namespace cppast;
using namespace cider::tool;

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
      std::cout << "cider version "
                << "1.0"
                << "\n";
      return 0;
    }

    const auto& paths = getFiles(options);
    if (paths.empty()) {
      print_error("missing file(s) argument");
      return 1;
    }

    std::thread([]() {
      std::this_thread::sleep_for(std::chrono::seconds(60));
      print_error("process guard timeout... terminating...");
      std::terminate();
    }).detach();

    // the compile config stores compilation flags
    const libclang_compile_config config = buildConfig(options, paths.back());

    // the logger is used to print diagnostics
    stderr_diagnostic_logger logger;
    if (options.count("verbose"))
      logger.set_verbose(true);

    cpp_entity_index idx;
    simple_file_parser<libclang_parser> parser(type_safe::ref(idx),
                                               default_logger());

    std::vector<const cppast::cpp_file*> files;

    if (options.count("integration_file")) {
      auto file =
          parser.parse(options["integration_file"].as<std::string>(), config);
      cppast::resolve_includes(parser, file.value(), config);
    }

    const auto genScope = options["namespace"].as<std::string>();

    for (const auto& path : paths) {
      auto file = parser.parse(path, config);
      if (file) {
        files.emplace_back(std::addressof(file.value()));
      }
      if (options.count("fatal_errors") == 1 && parser.error()) {
        return 1;
      }
    }

    const auto& metadata = collectMetadata(files);
    const auto& outDir = getOutputFilePath(options);

    if (options.count("swig")) {
      const auto moduleName = options["swig"].as<std::string>();
      if (moduleName.empty()) {
        print_error("missing swig module name argument");
      }

      std::string swig_dir;
      if (options.count("swig_directory")) {
        swig_dir = options["swig_directory"].as<std::string>();
        if (swig_dir.empty()) {
          print_error("missing swig module name argument");
        }
      } else {
        print_error("missing swig_directory argument");
        return 1;
      }

      const auto outSwigFilePath =
          getOutputFilePathWithoutExtension(moduleName, outDir);

      std::ofstream swigStream(outSwigFilePath + ".swig");
      printSwig(swigStream, outDir, swig_dir, moduleName, genScope, metadata,
                files);

      if (options.count("lua")) {
        const auto outLuaFilePath =
            getOutputFilePathWithoutExtension(moduleName, outDir);

        std::ofstream luaStream(outLuaFilePath + "_lua.cpp");
        lua_generator luaGen{luaStream, outDir, moduleName, metadata};

        for (const auto& file : files) {
          handleFile(luaGen, *file);
        }
      }
    }

    for (const auto& file : files) {
      const auto outFilePath =
          getOutputFilePathWithoutExtension(file->name(), outDir);

      std::ofstream headerStream(outFilePath + ".h");
      printHeader(headerStream, metadata, genScope, *file);

      std::ofstream sourceStream(outFilePath + ".cpp");
      printSource(sourceStream, metadata, genScope, outFilePath, *file);
    }
  } catch (const std::exception& ex) {
    print_error(std::string("[fatal parsing error] ") + ex.what());
    return 2;
  }
}
