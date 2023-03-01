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

namespace gunit {
namespace tool {

cxxopts::Options getOptions();

cppast::libclang_compile_config buildConfig(
    const cxxopts::ParseResult& options);

std::string getOutputFilePathWithoutExtension(
    const std::string& inputFilePath,
    const cxxopts::ParseResult& options);

}  // namespace tool
}  // namespace gunit