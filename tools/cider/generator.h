#pragma once

#include <iostream>

#include <cppast/cppast_fwd.hpp>

#include "ast_handler.h"
#include "metadata.h"
#include "namespaces_stack.h"

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace cider {
namespace tool {

struct generator : ast_handler {
  generator(std::ostream& out,
            std::string genScope,
            const MetadataStorage& metadata);

  void handleClass(const cppast::cpp_class& e, const bool enter) override;
  void handleNamespace(const cppast::cpp_entity& e, const bool enter) override;

 protected:
  namespaces_stack m_namespaces;
  const cppast::cpp_class* m_class =
      nullptr;  // TODO: stack to support nested classes

  std::ostream& m_out;
  const MetadataStorage& m_metadata;
};

struct header_generator final : public generator {
  using generator::generator;
  ~header_generator() override = default;

  void handleClass(const cppast::cpp_class& e, bool enter) override;
  void handleConstructor(const cppast::cpp_constructor& e,
                         cppast::cpp_access_specifier_kind kind) override;
  void handleMemberFunction(const cppast::cpp_member_function& e,
                            cppast::cpp_access_specifier_kind kind) override;
  void handleConversionOperator(
      const cppast::cpp_conversion_op& e,
      cppast::cpp_access_specifier_kind kind) override;
  void handleMemberVariable(const cppast::cpp_member_variable& e,
                            cppast::cpp_access_specifier_kind kind) override;
  void handleFreeFunction(const cppast::cpp_function& e) override;
  void handleEnumValue(const cppast::cpp_enum_value& e) override;
  void handleEnum(const cppast::cpp_enum& e, const bool enter) override;
  void handleFriend(const cppast::cpp_friend& e) override;
  void handleDestructor(const cppast::cpp_destructor& e,
                        cppast::cpp_access_specifier_kind kind) override;
};

struct source_generator final : public generator {
  using generator::generator;
  ~source_generator() override = default;

  void handleClass(const cppast::cpp_class& e, bool enter) override;
  void handleConstructor(const cppast::cpp_constructor& e,
                         cppast::cpp_access_specifier_kind kind) override;
  void handleFreeFunction(const cppast::cpp_function& e) override;
  void handleMemberFunction(const cppast::cpp_member_function& e,
                            cppast::cpp_access_specifier_kind kind) override;
  void handleConversionOperator(
      const cppast::cpp_conversion_op& e,
      cppast::cpp_access_specifier_kind kind) override;
  void handleFriend(const cppast::cpp_friend& e) override;
  void handleDestructor(const cppast::cpp_destructor& e,
                        cppast::cpp_access_specifier_kind kind) override;
};

}  // namespace tool
}  // namespace cider
