// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cppast/cppast_fwd.hpp>
#include <cppast/detail/intrusive_list.hpp>

#include "ast_handler.h"
#include "namespaces_stack.h"

#include <tlog.h>

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace cider {
namespace tool {

class MetadataStorage;

struct hash_generator final : ast_handler {
  hash_generator(std::ostream& out,
                 const std::string& outPath,
                 const MetadataStorage& metadata);
  ~hash_generator();

  void handleClass(const cppast::cpp_class& e,
                   cppast::cpp_access_specifier_kind /*kind*/,
                   bool enter) override;
  void handleMemberVariable(
      const cppast::cpp_member_variable& e,
      cppast::cpp_access_specifier_kind /*kind*/) override;
  void handleNamespace(const cppast::cpp_entity& e, const bool enter) override;
  void handleEnum(const cppast::cpp_enum& e, const bool enter) override;

 private:
  namespaces_stack m_namespaces;
  std::ostream& m_out;
  std::string m_outPath;
  const MetadataStorage& m_metadata;
  bool m_isAggregate = false;
  bool m_first = true;
};

}  // namespace tool
}  // namespace cider
