#pragma once

#include <cppast/cppast_fwd.hpp>
#include <cppast/detail/intrusive_list.hpp>

#include "ast_handler.h"

#include <iostream>
#include <unordered_set>

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace gunit {
namespace tool {

class MetadataStorage;

struct swig_generator final : ast_handler {
  swig_generator(std::ostream& out,
                 const std::string& module,
                 const MetadataStorage& metadata);
  ~swig_generator() = default;

  void finish();

 private:
  std::ostream& m_out;
  std::string m_module;
  const MetadataStorage& m_metadata;
};

void printSwig(
    std::ostream& os,
    const std::string& moduleName,
    const MetadataStorage& metadata,
    const cppast::detail::iteratable_intrusive_list<cppast::cpp_file>& files);

}  // namespace tool
}  // namespace gunit