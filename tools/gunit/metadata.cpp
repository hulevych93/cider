#include "metadata.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_enum.hpp>
#include <cppast/cpp_file.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>

#include "utils.h"

#include <assert.h>

using namespace cppast;

namespace gunit {
namespace tool {

namespace {

static class NullBuffer : public std::streambuf {
 public:
  int overflow(int c) { return c; }
} null_buffer;

static std::ostream null_stream(&null_buffer);

void collectParamType(std::unordered_set<std::string>& os,
                      const cpp_type& type) {
  std::string name;
  if (isUserDefined(type, name)) {
    os.emplace(name);
  }
}

void collectParamTypes(
    std::unordered_set<std::string>& os,
    const detail::iteratable_intrusive_list<cpp_function_parameter>& params) {
  for (const auto& param : params) {
    collectParamType(os, param.type());
  }
}

void collectBasesTypes(
    std::unordered_set<std::string>& os,
    const detail::iteratable_intrusive_list<cpp_base_class>& bases) {
  for (const auto& base : bases) {
    collectParamType(os, base.type());
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
    m_file->exports.emplace(m_namespaces.scope() + "::" + e.name());
    m_storage.enums.emplace(m_namespaces.scope() + "::" + e.name());
  }
}

void metadata_collector::handleFile(const cppast::cpp_file& e,
                                    const bool enter) {
  if (enter) {
    assert(!m_file.has_value());
    m_file = FileMetadata{};
    m_file->name = e.name();
  } else {
    assert(m_file.has_value());
    m_storage.files.emplace(e.name(), m_file.value());
    m_file.reset();
  }
}

void metadata_collector::handleClass(const cppast::cpp_class& e, bool enter) {
  if (enter) {
    assert(!m_class.has_value());
    m_class = ClassMetadata{};
    m_class->name = e.name();
    m_class->file = m_file->name;
    m_file->exports.emplace(m_namespaces.scope() + "::" + e.name());
    collectBasesTypes(m_file->imports, e.bases());
  } else {
    assert(m_class.has_value());
    m_storage.classes.emplace(m_namespaces.scope() + "::" + e.name(),
                              m_class.value());
    m_class.reset();
  }
}

void metadata_collector::handleConstructor(const cppast::cpp_constructor& e) {
  assert(m_class.has_value());

  const bool isCopy = isCopyContructor(e);
  const bool isMove = isMoveContructor(e);

  m_class->hasUserConstructors |= (!isCopy && !isMove);
  m_class->hasAnyMethods = true;
  m_class->hasCopyConstructor |= isCopy;
  m_class->hasMoveConstructor |= isMove;

  collectParamTypes(m_file->imports, e.parameters());
}

void metadata_collector::handleMemberFunction(
    const cppast::cpp_member_function& e) {
  assert(m_class.has_value());
  m_class->hasAnyMethods = true;
  m_class->hasCopyAssignmentOperator |= isCopyAssignmentOperator(e);
  m_class->hasMoveAssignmentOperator |= isMoveAssignmentOperator(e);

  if (e.is_virtual()) {
    const auto& info = e.virtual_info().value();
    m_class->isAbstract |= info.is_set(cppast::cpp_virtual_flags::pure);
  }

  collectParamTypes(m_file->imports, e.parameters());
  collectParamType(m_file->imports, e.return_type());
}

void metadata_collector::handleFreeFunction(const cppast::cpp_function& e) {
  collectParamTypes(m_file->imports, e.parameters());
  collectParamType(m_file->imports, e.return_type());
}

void metadata_collector::handleMemberVariable(
    const cppast::cpp_member_variable& e) {
  assert(m_class.has_value());
  m_class->hasAnyFields = true;
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
    const cppast::detail::iteratable_intrusive_list<cppast::cpp_file>& files) {
  MetadataStorage metadata;
  metadata_collector collector(metadata);

  for (const auto& file : files) {
    handleFile(collector, file);
  }

  collector.finish();
  return metadata;
}

}  // namespace tool
}  // namespace gunit
