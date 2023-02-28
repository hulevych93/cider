#pragma once

#include <iostream>

#include <cppast/cppast_fwd.hpp>

#include "namespaces_stack.h"

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace gunit {
namespace tool {

struct generator {
  generator(std::ostream& out, const cppast::cpp_entity_index& idx);

  void handleAccess(const cppast::cpp_entity& e);
  void handleClass(const cppast::cpp_class& e, const bool enter);
  void handleNamespace(const cppast::cpp_entity& e, const bool enter);
  void handleMemberVariable(const cppast::cpp_member_variable& e);

 protected:
  namespaces_stack m_namespaces;
  const cppast::cpp_class* m_class =
      nullptr;  // TODO: stack to support nested classes

  std::ostream& m_out;
  const cppast::cpp_entity_index& m_idx;
};

struct header_generator final : public generator {
  header_generator(std::ostream& out, const cppast::cpp_entity_index& idx);
  ~header_generator() = default;

  void handleClass(const cppast::cpp_class& e, const bool enter);
  void handleConstructor(const cppast::cpp_constructor& e);
  void handleAccess(const cppast::cpp_entity& e);
  void handleFreeFunction(const cppast::cpp_function& e);
  void handleMemberFunction(const cppast::cpp_member_function& e);
  void handleMemberVariable(const cppast::cpp_member_variable& e);

 private:
  std::string m_access = "public";
};

struct source_generator final : public generator {
  source_generator(std::ostream& out, const cppast::cpp_entity_index& idx);
  ~source_generator() = default;

  void handleConstructor(const cppast::cpp_constructor& e);
  void handleFreeFunction(const cppast::cpp_function& e);
  void handleMemberFunction(const cppast::cpp_member_function& e);
};

}  // namespace tool
}  // namespace gunit
