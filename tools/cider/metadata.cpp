// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "metadata.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_enum.hpp>
#include <cppast/cpp_file.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>

#include "utils.h"

#include <assert.h>

using namespace cppast;

namespace cider {
namespace tool {

namespace {

static class NullBuffer : public std::streambuf {
 public:
  int overflow(int c) { return c; }
} null_buffer;

static std::ostream null_stream(&null_buffer);

void collectParamType(std::unordered_set<std::string>& os,
                      const std::string& scope,
                      const cpp_type& type) {
  std::string name;
  if (isUserDefined(type, scope, name)) {
    os.emplace(name);
  }
}

void collectParamTypes(
    std::unordered_set<std::string>& os,
    const std::string& scope,
    const detail::iteratable_intrusive_list<cpp_function_parameter>& params) {
  for (const auto& param : params) {
    collectParamType(os, scope, param.type());
  }
}

void collectBasesTypes(
    std::unordered_set<std::string>& os,
    const std::string& scope,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases) {
  for (const auto& base : bases) {
    collectParamType(os, scope, base.type());
  }
}

void collectBases(
    std::unordered_set<std::string>& os,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases) {
  for (const auto& base : bases) {
    os.emplace(base.name());
  }
}

}  // namespace

void metadata_collector::handleNamespace(const cppast::cpp_entity& e,
                                         const bool enter) {
  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

void metadata_collector::handleEnum(const cppast::cpp_enum& e,
                                    const bool enter) {
  if (enter) {
    m_fileMetadata->exports.emplace(m_namespaces.nativeScope() +
                                    "::" + e.name());
    m_storage.enums.emplace(m_namespaces.nativeScope() + "::" + e.name());
    m_fileMetadata->hasAggregatesOrEnums = true;
  }
}

void metadata_collector::handleFile(const cppast::cpp_file& e,
                                    const bool enter) {
  if (enter) {
    assert(!m_fileMetadata.has_value());
    m_fileMetadata = FileMetadata{};
    m_fileMetadata->name = e.name();
  } else {
    assert(m_fileMetadata.has_value());
    m_storage.files.emplace(e.name(), m_fileMetadata.value());
    m_fileMetadata.reset();
  }
}

void metadata_collector::handleClass(const cppast::cpp_class& e, bool enter) {
  if (enter) {
    m_classes.push_back(std::addressof(e));
  } else {
    m_classes.pop_back();
  }
  if (e.is_declaration()) {
    return;
  }
  if (isException(e)) {
    return;
  }
  if (enter) {
    assert(!m_classMetadata.has_value());
    m_classMetadata = ClassMetadata{};
    m_classMetadata->name = e.name();
    m_classMetadata->file = m_fileMetadata->name;
    collectBases(m_classMetadata->bases, e.bases());
    m_fileMetadata->exports.emplace(m_namespaces.nativeScope() +
                                    "::" + e.name());
    collectBasesTypes(m_fileMetadata->imports, m_namespaces.nativeScope(),
                      e.bases());
  } else {
    assert(m_classMetadata.has_value());
    m_storage.classes.emplace(m_namespaces.nativeScope() + "::" + e.name(),
                              m_classMetadata.value());
    m_classMetadata.reset();
  }
}

void metadata_collector::handleConstructor(
    const cppast::cpp_constructor& e,
    const cppast::cpp_access_specifier_kind kind) {
  assert(m_classMetadata.has_value());

  const bool isCopy = isCopyContructor(e);
  const bool isMove = isMoveContructor(e);

  m_classMetadata->hasUserConstructors |= (!isCopy && !isMove);
  m_classMetadata->hasCopyConstructor |= isCopy;
  m_classMetadata->hasMoveConstructor |= isMove;

  collectParamTypes(m_fileMetadata->imports, m_namespaces.nativeScope(),
                    e.parameters());
}

void metadata_collector::handleMemberFunction(
    const cppast::cpp_member_function& e,
    const cppast::cpp_access_specifier_kind kind) {
  assert(m_classMetadata.has_value());

  const auto name = e.name();
  m_classMetadata->hasCopyAssignmentOperator |=
      isCopyAssignmentOperator(*m_classes.back(), e);
  m_classMetadata->hasMoveAssignmentOperator |=
      isMoveAssignmentOperator(*m_classes.back(), e);

  if (e.is_virtual()) {
    const auto& info = e.virtual_info().value();
    m_classMetadata->isAbstract |= info.is_set(cppast::cpp_virtual_flags::pure);
  }

  m_classMetadata->hasPrivateMethods |=
      kind == cppast::cpp_access_specifier_kind::cpp_private;
  m_classMetadata->hasPublicMethods |=
      kind == cppast::cpp_access_specifier_kind::cpp_public;

  collectParamTypes(m_fileMetadata->imports, m_namespaces.nativeScope(),
                    e.parameters());
  collectParamType(m_fileMetadata->imports, m_namespaces.nativeScope(),
                   e.return_type());
}

void metadata_collector::handleFreeFunction(const cppast::cpp_function& e) {
  collectParamTypes(m_fileMetadata->imports, m_namespaces.nativeScope(),
                    e.parameters());
  collectParamType(m_fileMetadata->imports, m_namespaces.nativeScope(),
                   e.return_type());
}

void metadata_collector::handleMemberVariable(
    const cppast::cpp_member_variable& e,
    const cppast::cpp_access_specifier_kind kind) {
  assert(m_classMetadata.has_value());
  m_classMetadata->fieldNames.emplace(e.name());
  m_fileMetadata->hasAggregatesOrEnums = true;  // TODO

  m_classMetadata->hasPrivateFields |=
      kind == cppast::cpp_access_specifier_kind::cpp_private;
  m_classMetadata->hasPublicFields |=
      kind == cppast::cpp_access_specifier_kind::cpp_public;
  m_classMetadata->hasProtectedFields |=
      kind == cppast::cpp_access_specifier_kind::cpp_protected;
}

void metadata_collector::finish() {
  auto& files = m_storage.files;
  for (auto& fileIt : files) {
    auto& file = fileIt.second;
    for (const auto& exportClass : file.exports) {
      file.imports.erase(exportClass);
    }
  }
}

MetadataStorage collectMetadata(
    const std::vector<const cppast::cpp_file*>& files) {
  MetadataStorage metadata;
  metadata_collector collector(metadata);

  for (const auto& file : files) {
    handleFile(collector, *file, false);
  }

  collector.finish();
  std::cerr << metadata;
  return metadata;
}

std::ostream& operator<<(std::ostream& os, const ClassMetadata& metadata) {
  os << "{";
  os << metadata.hasCopyAssignmentOperator << ", ";
  os << metadata.hasCopyConstructor << ", ";
  os << metadata.hasMoveAssignmentOperator << ", ";
  os << metadata.hasUserConstructors;
  os << "}" << std::endl;

  return os;
}

std::ostream& operator<<(std::ostream& os, const MetadataStorage& metadata) {
  os << "[";
  for (const auto& [entry, classData] : metadata.classes) {
    os << entry << ": " << classData << ", ";
  }
  os << "]" << std::endl;

  return os;
}

}  // namespace tool
}  // namespace cider
