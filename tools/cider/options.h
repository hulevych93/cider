// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cppast/cppast_fwd.hpp>
#include <string>

namespace cxxopts {
class Options;
class ParseResult;
}  // namespace cxxopts

namespace cppast {
class libclang_compile_config;
}  // namespace cppast

namespace cider {
namespace tool {

cxxopts::Options getOptions();

cppast::libclang_compile_config buildConfig(const cxxopts::ParseResult& options,
                                            const std::string path);

std::string getOutputFilePath(const cxxopts::ParseResult& options);

std::string getOutputFilePathWithoutExtension(const std::string& inputFilePath,
                                              const std::string& outDir);

}  // namespace tool
}  // namespace cider
