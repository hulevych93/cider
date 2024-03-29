// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cppast/cppast_fwd.hpp>

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace cider {
namespace tool {

struct ast_handler {
  virtual ~ast_handler() = default;

  virtual void handleFile(const cppast::cpp_file& /*e*/, const bool /*enter*/) {
  }
  virtual void handleNamespace(const cppast::cpp_entity& /*e*/,
                               const bool /*enter*/) {}
  virtual void handleClass(const cppast::cpp_class& /*e*/,
                           cppast::cpp_access_specifier_kind /*kind*/,
                           bool /*enter*/) {}
  virtual void handleConstructor(const cppast::cpp_constructor& /*e*/,
                                 cppast::cpp_access_specifier_kind /*kind*/) {}
  virtual void handleMemberFunction(
      const cppast::cpp_member_function& /*e*/,
      cppast::cpp_access_specifier_kind /*kind*/) {}
  virtual void handleDestructor(const cppast::cpp_destructor& /*e*/,
                                cppast::cpp_access_specifier_kind /*kind*/) {}
  virtual void handleConversionOperator(
      const cppast::cpp_conversion_op& /*e*/,
      cppast::cpp_access_specifier_kind /*kind*/) {}
  virtual void handleMemberVariable(
      const cppast::cpp_member_variable& /*e*/,
      cppast::cpp_access_specifier_kind /*kind*/) {}
  virtual void handleFreeFunction(const cppast::cpp_function& /*e*/) {}
  virtual void handleEnumValue(const cppast::cpp_enum_value& /*e*/) {}
  virtual void handleEnum(const cppast::cpp_enum& /*e*/, const bool /*enter*/) {
  }
  virtual void handleFriend(const cppast::cpp_friend& /*e*/) {}
  virtual void handleVariable(const cppast::cpp_variable& /*e*/) {}
  virtual void handleAlias(const cppast::cpp_type_alias& /*e*/) {}
};

}  // namespace tool
}  // namespace cider
