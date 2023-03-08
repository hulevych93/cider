#pragma once

#include <iostream>

#include <cppast/cppast_fwd.hpp>

#include "ast_handler.h"
#include "metadata.h"
#include "namespaces_stack.h"

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace gunit {
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
  void handleConstructor(const cppast::cpp_constructor& e) override;
  void handleMemberFunction(const cppast::cpp_member_function& e) override;
  void handleMemberVariable(const cppast::cpp_member_variable& e) override;
  void handleFreeFunction(const cppast::cpp_function& e) override;
};

struct source_generator final : public generator {
  using generator::generator;
  ~source_generator() override = default;

  void handleConstructor(const cppast::cpp_constructor& e) override;
  void handleFreeFunction(const cppast::cpp_function& e) override;
  void handleMemberFunction(const cppast::cpp_member_function& e) override;
};

}  // namespace tool
}  // namespace gunit
