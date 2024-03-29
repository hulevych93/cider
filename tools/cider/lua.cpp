// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "lua.h"

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

void printAggregateField(std::ostream& os, const std::string& name) {
  os << "code += \"{var}." + name;
  os << " = \" + visitor(aggregate." + name + ")";
  os << R"(+ "\n";)"
     << "\n";
}

void printBaseClassesAggregateFields(
    std::ostream& os,
    const MetadataStorage& metadata,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases,
    const std::string& scope) {
  for (const auto& base : bases) {
    const auto it = metadata.classes.find(scope + "::" + base.name());
    if (it != metadata.classes.end()) {
      const auto& fields = it->second.fieldNames;
      for (const auto& field : fields) {
        printAggregateField(os, field);
      }
    }
  }
}

void printAggregateProducer(std::ostream& os,
                            const MetadataStorage& metadata,
                            const std::string& module,
                            const std::string& scope,
                            const cpp_class& e,
                            const bool enter) {
  if (enter) {
    os << "template <>\n";
    os << "std::string produceAggregateCode(const std::string& moduleName, "
          "const "
       << scope + "::" + e.name() << "& aggregate, ";
    os << "CodeSink& sink) {\n";
    os << "lua::ParamVisitor visitor(moduleName);\n";
    os << "std::string code;\n";
    os << "code += \"{var} = " + module + "." + e.name() + R"(()\n";)"
       << "\n";
    printBaseClassesAggregateFields(os, metadata, e.bases(), scope);
  } else {
    os << "return sink.processLocalVar(std::move(code));\n";
    os << "}\n\n";
  }
}

void printEnumProducer(std::ostream& os,
                       const std::string& scope,
                       const std::string& name,
                       const bool enter) {
  if (enter) {
    os << "template <>\n";
    os << "std::string produceAggregateCode(const std::string&, const "
       << scope + "::" + name << "& aggregate, ";
    os << "CodeSink&) {\n";
    os << "switch (aggregate) {\n";
  } else {
    os << "}\n";
    os << "return {};\n";
    os << "}\n\n";
  }
}

void printEnumField(std::ostream& os,
                    const std::string& module,
                    const std::string& enumName,
                    const std::string& scope,
                    const std::string& name,
                    const bool isScoped) {
  os << "case " << scope + "::" << name << ":\n";
  os << "return \"" + module + "." + enumName + "(" + module << ".";
  if (isScoped) {
    os << enumName + "_";
  }
  os << name << ")\";\n";
  os << "break;\n";
}

}  // namespace

lua_generator::lua_generator(std::ostream& out,
                             const std::string& outPath,
                             const std::string& module,
                             const MetadataStorage& metadata)
    : m_out(out), m_outPath(outPath), m_module(module), m_metadata(metadata) {
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

lua_generator::~lua_generator() {
  m_out << "} // namespace cider\n";
  m_out << "} // namespace recorder\n\n";
}

void lua_generator::handleClass(const cppast::cpp_class& e,
                                cppast::cpp_access_specifier_kind /*kind*/,
                                bool enter) {
  m_isAggregate = isAggregate(e.name(), m_namespaces.nativeScope(), m_metadata);
  if (!m_isAggregate) {
    return;
  }

  if (e.is_declaration()) {
    return;
  }

  printAggregateProducer(m_out, m_metadata, m_module,
                         m_namespaces.nativeScope(), e, enter);
}

void lua_generator::handleMemberVariable(
    const cppast::cpp_member_variable& e,
    cppast::cpp_access_specifier_kind /*kind*/) {
  if (!m_isAggregate) {
    return;
  }

  printAggregateField(m_out, e.name());
}

void lua_generator::handleNamespace(const cpp_entity& e, const bool enter) {
  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

void lua_generator::handleEnumValue(const cppast::cpp_enum_value& e) {
  printEnumField(m_out, m_module, m_namespaces.top(),
                 m_namespaces.nativeScope(), e.name(), m_isScopedEnum);
}

void lua_generator::handleEnum(const cppast::cpp_enum& e, const bool enter) {
  printEnumProducer(m_out, m_namespaces.nativeScope(), e.name(), enter);
  m_isScopedEnum = e.is_scoped();

  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

}  // namespace tool
}  // namespace cider
