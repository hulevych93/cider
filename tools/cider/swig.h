// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cppast/cppast_fwd.hpp>
#include <cppast/detail/intrusive_list.hpp>

#include "ast_handler.h"
#include "namespaces_stack.h"

#include <iostream>

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace cider {
namespace tool {

class MetadataStorage;

struct swig_generator final : ast_handler {
  swig_generator(std::ostream& out,
                 const std::string& outPath,
                 const std::string& swigDir,
                 const std::string& module,
                 const std::string& genScope,
                 const MetadataStorage& metadata);
  ~swig_generator() = default;

  void handleNamespace(const cppast::cpp_entity& e, const bool enter) override;
  void handleEnum(const cppast::cpp_enum& e, const bool enter) override;

  void finish();

 private:
  std::ostream& m_out;
  std::string m_outPath;
  std::string m_swigDir;
  std::string m_module;
  const MetadataStorage& m_metadata;
  namespaces_stack m_namespaces;
};

void printSwig(std::ostream& os,
               const std::string& outPath,
               const std::string& swigDir,
               const std::string& moduleName,
               const std::string& genScope,
               const MetadataStorage& metadata,
               const std::vector<const cppast::cpp_file*>& files);

}  // namespace tool
}  // namespace cider
