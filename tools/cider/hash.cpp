// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "hash.h"

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

void printAggregateHashFields(std::ostream& os,
                              const std::string& name,
                              bool first) {
  if (first) {
    os << "  size_t seed = std::hash<decltype(aggregate." << name
       << ")>{}(aggregate." << name << ");\n";
  } else {
    os << "  std::hash_combine(seed, aggregate." << name << ");\n";
  }
}

template <typename F>
bool printBaseClassesHashFields(
    std::ostream& os,
    const MetadataStorage& metadata,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases,
    const std::string& scope,
    F&& func) {
  auto first = true;
  for (const auto& base : bases) {
    const auto it = metadata.classes.find(scope + "::" + base.name());
    if (it != metadata.classes.end()) {
      const auto& fields = it->second.fieldNames;
      for (const auto& field : fields) {
        func(os, field, first);
        first = false;
      }
    }
  }
  return first;
}

bool printAggregateHash(std::ostream& os,
                        const MetadataStorage& metadata,
                        const std::string& scope,
                        const cpp_class& e,
                        const bool enter) {
  if (enter) {
    os << "template <>\n";
    os << "size_t hashAggregate(const " << scope + "::" + e.name()
       << "& aggregate) {\n";
    return printBaseClassesHashFields(os, metadata, e.bases(), scope,
                                      printAggregateHashFields);
  } else {
    os << "  return seed;\n";
    os << "}\n\n";
  }
  return true;
}

void printEnumHash(std::ostream& os,
                   const std::string& scope,
                   const std::string& name,
                   const bool enter) {
  if (enter) {
    os << "template <>\n";
    os << "size_t hashAggregate(const " << scope + "::" + name
       << "& aggregate) {\n";
    os << "size_t seed = static_cast<int>(aggregate);\n";
  } else {
    os << "  return seed;\n";
    os << "}\n\n";
  }
}

}  // namespace

hash_generator::hash_generator(std::ostream& out,
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

hash_generator::~hash_generator() {
  m_out << "} // namespace cider\n";
  m_out << "} // namespace recorder\n\n";
}

void hash_generator::handleClass(const cppast::cpp_class& e,
                                 cppast::cpp_access_specifier_kind /*kind*/,
                                 bool enter) {
  m_isAggregate = isAggregate(e.name(), m_namespaces.nativeScope(), m_metadata);
  if (!m_isAggregate) {
    return;
  }

  if (e.is_declaration()) {
    return;
  }

  if (enter) {
    m_first = true;
  }
  m_first = printAggregateHash(m_out, m_metadata, m_namespaces.nativeScope(), e,
                               enter);
}

void hash_generator::handleMemberVariable(
    const cppast::cpp_member_variable& e,
    cppast::cpp_access_specifier_kind /*kind*/) {
  if (!m_isAggregate) {
    return;
  }

  printAggregateHashFields(m_out, e.name(), m_first);
  m_first = false;
}

void hash_generator::handleEnum(const cppast::cpp_enum& e, const bool enter) {
  printEnumHash(m_out, m_namespaces.nativeScope(), e.name(), enter);

  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

void hash_generator::handleNamespace(const cpp_entity& e, const bool enter) {
  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

}  // namespace tool
}  // namespace cider
