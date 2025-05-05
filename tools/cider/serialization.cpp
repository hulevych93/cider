// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "serialization.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_enum.hpp>
#include <cppast/cpp_member_variable.hpp>

#include "options.h"
#include "utils.h"

using namespace cppast;

namespace cider {
namespace tool {

namespace {

static class NullBuffer : public std::streambuf {
 public:
  int overflow(int c) { return c; }
} null_buffer;

static std::ostream null_stream(&null_buffer);

void printAggregateSerializeField(std::ostream& os, const std::string& name) {
  os << "serializer << aggregate." << name << ";\n";
}

template <typename F>
void printBaseClassesAggregateFields(
    std::ostream& os,
    const MetadataStorage& metadata,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases,
    const std::string& scope,
    F&& func) {
  for (const auto& base : bases) {
    const auto it = metadata.classes.find(scope + "::" + base.name());
    if (it != metadata.classes.end()) {
      const auto& fields = it->second.fieldNames;
      for (const auto& field : fields) {
        func(os, field);
      }
    }
  }
}

void printAggregateSerializer(std::ostream& os,
                              const MetadataStorage& metadata,
                              const std::string& scope,
                              const cpp_class& e,
                              const bool enter) {
  if (enter) {
    os << "template <>\n";
    os << "bool serializeAggregate(const " << scope + "::" + e.name()
       << "& aggregate, serialization::Serializer& serializer) {\n";
    printBaseClassesAggregateFields(os, metadata, e.bases(), scope,
                                    printAggregateSerializeField);
  } else {
    os << "return true;\n";
    os << "}\n\n";
  }
}

void printEnumSerializer(std::ostream& os,
                         const std::string& scope,
                         const std::string& name,
                         const bool enter) {
  if (enter) {
    os << "template <>\n";
    os << "bool serializeAggregate(const " << scope + "::" + name
       << "& aggregate, serialization::Serializer& serializer) {\n";
    os << "serializer << aggregate;\n";
  } else {
    os << "return true;\n";
    os << "}\n\n";
  }
}

}  // namespace

serialization_generator::serialization_generator(
    std::ostream& out,
    const std::string& outPath,
    const MetadataStorage& metadata)
    : m_out(out), m_outPath(outPath), m_metadata(metadata) {
  const auto& files = metadata.files;
  for (const auto& [name, file] : files) {
    if (file.hasAggregatesOrEnums) {
      m_out << "#include \"" << name << "\"\n";
    }
  }

  m_out << "#include \"recorder/details/lua/lua_params.h\"\n\n";
  m_out << "namespace cider {\n";
  m_out << "namespace recorder {\n\n";
}

serialization_generator::~serialization_generator() {
  m_out << "} // namespace cider\n";
  m_out << "} // namespace recorder\n\n";
}

void serialization_generator::handleClass(
    const cppast::cpp_class& e,
    cppast::cpp_access_specifier_kind /*kind*/,
    bool enter) {
  m_isAggregate = isAggregate(e.name(), m_namespaces.nativeScope(), m_metadata);
  if (!m_isAggregate) {
    return;
  }

  if (e.is_declaration()) {
    return;
  }

  printAggregateSerializer(m_out, m_metadata, m_namespaces.nativeScope(), e,
                           enter);
}

void serialization_generator::handleMemberVariable(
    const cppast::cpp_member_variable& e,
    cppast::cpp_access_specifier_kind /*kind*/) {
  if (!m_isAggregate) {
    return;
  }

  printAggregateSerializeField(m_out, e.name());
}

void serialization_generator::handleEnum(const cppast::cpp_enum& e,
                                         const bool enter) {
  printEnumSerializer(m_out, m_namespaces.nativeScope(), e.name(), enter);

  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

void serialization_generator::handleNamespace(const cpp_entity& e,
                                              const bool enter) {
  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

}  // namespace tool
}  // namespace cider
