// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "mutator.h"

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

void printAggregateFieldMutator(std::ostream& os, const std::string& name) {
  os << "  mutator(aggregate." + name + ");\n";
}

void printBaseClassesAggregateFields(
    std::ostream& os,
    const MetadataStorage& metadata,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases,
    const std::string& scope,
    void (*func)(std::ostream&, const std::string&)) {
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

void printAggregateMutator(std::ostream& os,
                           const MetadataStorage& metadata,
                           const std::string& scope,
                           const cpp_class& e,
                           const bool enter) {
  if (enter) {
    os << "template <>\n";
    os << "void mutateAggregate(const IParamMutator& mutator, "
       << scope + "::" + e.name() << "& aggregate) {\n";
    printBaseClassesAggregateFields(os, metadata, e.bases(), scope,
                                    printAggregateFieldMutator);
  } else {
    os << "}\n\n";
  }
}

void printEnumMutator(std::ostream& os,
                      const std::string& scope,
                      const std::string& name,
                      const bool enter) {
  if (enter) {
    os << "template <>\n";
    os << "void mutateAggregate(const IParamMutator& mutator, "
       << scope + "::" + name << "& aggregate) {\n";
    os << "std::vector<" << scope + "::" + name << "> vals;\n";
  } else {
    os << "const auto rnd = mutator.random(vals.size());\n";
    os << "if(rnd.has_value()) {\n";
    os << "  aggregate = vals[rnd.value()];\n";
    os << "}\n}\n\n";
  }
}

void printEnumFieldMutator(std::ostream& os,
                           const std::string&,
                           const std::string& scope,
                           const std::string& name) {
  os << "vals.emplace_back(" << scope << "::" << name << ");\n";
}

}  // namespace

mutator_generator::mutator_generator(std::ostream& out,
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

mutator_generator::~mutator_generator() {
  m_out << "} // namespace cider\n";
  m_out << "} // namespace recorder\n\n";
}

void mutator_generator::handleClass(const cppast::cpp_class& e, bool enter) {
  m_isAggregate = isAggregate(e.name(), m_namespaces.nativeScope(), m_metadata);
  if (!m_isAggregate) {
    return;
  }

  printAggregateMutator(m_out, m_metadata, m_namespaces.nativeScope(), e,
                        enter);
}

void mutator_generator::handleMemberVariable(
    const cppast::cpp_member_variable& e,
    cppast::cpp_access_specifier_kind /*kind*/) {
  if (!m_isAggregate) {
    return;
  }

  printAggregateFieldMutator(m_out, e.name());
}

void mutator_generator::handleNamespace(const cpp_entity& e, const bool enter) {
  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

void mutator_generator::handleEnumValue(const cppast::cpp_enum_value& e) {
  printEnumFieldMutator(m_out, m_namespaces.top(), m_namespaces.nativeScope(),
                        e.name());
}

void mutator_generator::handleEnum(const cppast::cpp_enum& e,
                                   const bool enter) {
  printEnumMutator(m_out, m_namespaces.nativeScope(), e.name(), enter);

  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

}  // namespace tool
}  // namespace cider
