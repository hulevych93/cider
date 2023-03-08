#pragma once

#include <cppast/cppast_fwd.hpp>

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace gunit {
namespace tool {

struct ast_handler {
  virtual ~ast_handler() = default;

  virtual void handleFile(const cppast::cpp_file& /*e*/, const bool /*enter*/) {
  }
  virtual void handleNamespace(const cppast::cpp_entity& /*e*/,
                               const bool /*enter*/) {}
  virtual void handleClass(const cppast::cpp_class& /*e*/, bool /*enter*/) {}
  virtual void handleConstructor(const cppast::cpp_constructor& /*e*/) {}
  virtual void handleMemberFunction(const cppast::cpp_member_function& /*e*/) {}
  virtual void handleMemberVariable(const cppast::cpp_member_variable& /*e*/) {}
  virtual void handleFreeFunction(const cppast::cpp_function& /*e*/) {}
};

}  // namespace tool
}  // namespace gunit
