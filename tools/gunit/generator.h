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
  generator(std::ostream& out, const MetadataStorage& metadata);

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
  header_generator(std::ostream& out, const MetadataStorage& metadata);
  ~header_generator() = default;

  void handleClass(const cppast::cpp_class& e, bool enter) override;
  void handleConstructor(const cppast::cpp_constructor& e) override;
  void handleMemberFunction(const cppast::cpp_member_function& e) override;
  void handleMemberVariable(const cppast::cpp_member_variable& e) override;
  void handleFreeFunction(const cppast::cpp_function& e) override;
};

struct source_generator final : public generator {
  source_generator(std::ostream& out, const MetadataStorage& metadata);
  ~source_generator() = default;

  void handleConstructor(const cppast::cpp_constructor& e) override;
  void handleFreeFunction(const cppast::cpp_function& e) override;
  void handleMemberFunction(const cppast::cpp_member_function& e) override;
};

void process_file(ast_handler& handler, const cppast::cpp_file& file);

void printHeader(const std::string& outputFile,
                 const MetadataStorage& metadata,
                 const cppast::cpp_file& file);

void printSource(const std::string& outputFile,
                 const MetadataStorage& metadata,
                 const cppast::cpp_file& file);

}  // namespace tool
}  // namespace gunit
