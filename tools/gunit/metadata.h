#pragma once

#include <cppast/cppast_fwd.hpp>

#include "ast_handler.h"

#include <optional>
#include <unordered_map>
#include "namespaces_stack.h"

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace gunit {
namespace tool {

struct ClassMetadata final {
  bool hasAnyMethods = false;  // (e.g. public ones)
  bool hasAnyFields = false;   // (e.g. public ones)
  bool hasUserConstructors =
      false;  // (e.g. will compiler generate default one)

  // bool hasCopyConstructor = false;
  // bool hasMoveConstructor = false;
  // bool hasCopyOperator = false;
  // bool hasMoveOperator = false;
};

using MetadataStorage = std::unordered_map<std::string, ClassMetadata>;

struct metadata_collector final : ast_handler {
  metadata_collector(MetadataStorage& out) : m_storage(out) {}
  ~metadata_collector() = default;

  void handleClass(const cppast::cpp_class& e, bool enter) override;
  void handleConstructor(const cppast::cpp_constructor& e) override;
  void handleMemberFunction(const cppast::cpp_member_function& e) override;
  void handleMemberVariable(const cppast::cpp_member_variable& e) override;
  void handleNamespace(const cppast::cpp_entity& e, const bool enter) override;

 private:
  MetadataStorage& m_storage;
  namespaces_stack m_namespaces;

  std::optional<ClassMetadata> m_current;  // TODO: support nested classes?
};

}  // namespace tool
}  // namespace gunit
