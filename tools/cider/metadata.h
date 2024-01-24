#pragma once

#include <cppast/cppast_fwd.hpp>
#include <cppast/detail/intrusive_list.hpp>

#include "ast_handler.h"

#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "namespaces_stack.h"

namespace cppast {
class cpp_member_variable;
}  // namespace cppast

namespace cider {
namespace tool {

struct ClassMetadata final {
  std::string name;
  std::string file;

  std::unordered_set<std::string> bases;  // (e.g. public ones)

  bool isAbstract = false;  // e.g. has any pure virtual methods

  bool hasPublicMethods = false;
  bool hasPrivateMethods = false;

  bool hasPublicFields = false;
  bool hasPrivateFields = false;
  bool hasProtectedFields = false;

  std::set<std::string> fieldNames;  // (e.g. public ones)

  bool hasUserConstructors =
      false;  // (e.g. will compiler generate default one)
  bool hasCopyConstructor = false;
  bool hasMoveConstructor = false;
  bool hasCopyAssignmentOperator = false;
  bool hasMoveAssignmentOperator = false;
};

std::ostream& operator<<(std::ostream& os, const ClassMetadata& metadata);

struct FileMetadata final {
  std::string name;
  bool hasAggregatesOrEnums = false;

  std::unordered_set<std::string> exports;
  std::unordered_set<std::string> imports;
};

struct MetadataStorage final {
  std::unordered_set<std::string> enums;
  std::unordered_map<std::string, ClassMetadata> classes;
  std::unordered_map<std::string, FileMetadata> files;
};

std::ostream& operator<<(std::ostream& os, const MetadataStorage& metadata);

struct metadata_collector final : ast_handler {
  metadata_collector(MetadataStorage& out) : m_storage(out) {}
  ~metadata_collector() = default;

  void handleFile(const cppast::cpp_file& e, bool enter) override;
  void handleClass(const cppast::cpp_class& e, bool enter) override;
  void handleConstructor(const cppast::cpp_constructor& e,
                         cppast::cpp_access_specifier_kind kind) override;
  void handleMemberFunction(const cppast::cpp_member_function& e,
                            cppast::cpp_access_specifier_kind kind) override;
  void handleFreeFunction(const cppast::cpp_function& e) override;
  void handleMemberVariable(const cppast::cpp_member_variable& e,
                            cppast::cpp_access_specifier_kind kind) override;
  void handleNamespace(const cppast::cpp_entity& e, const bool enter) override;
  void handleEnum(const cppast::cpp_enum& e, const bool enter) override;

  void finish();

 private:
  MetadataStorage& m_storage;
  namespaces_stack m_namespaces;

  std::optional<ClassMetadata> m_class;  // TODO: support nested classes?
  std::optional<FileMetadata> m_file;
};

MetadataStorage collectMetadata(
    const std::vector<const cppast::cpp_file*>& files);

}  // namespace tool
}  // namespace cider
