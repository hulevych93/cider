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

struct lua_generator final : ast_handler {
  lua_generator(std::ostream& out,
                const std::string& outPath,
                const std::string& module,
                const MetadataStorage& metadata);
  ~lua_generator();

  void handleClass(const cppast::cpp_class& e,
                   cppast::cpp_access_specifier_kind /*kind*/,
                   bool enter) override;
  void handleMemberVariable(
      const cppast::cpp_member_variable& e,
      cppast::cpp_access_specifier_kind /*kind*/) override;
  void handleEnumValue(const cppast::cpp_enum_value& e) override;
  void handleEnum(const cppast::cpp_enum& e, const bool enter) override;
  void handleNamespace(const cppast::cpp_entity& e, const bool enter) override;

 private:
  namespaces_stack m_namespaces;
  std::ostream& m_out;
  std::string m_outPath;
  std::string m_module;
  const MetadataStorage& m_metadata;
  bool m_isAggregate = false;
};

}  // namespace tool
}  // namespace cider
