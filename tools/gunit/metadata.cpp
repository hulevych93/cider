#include "metadata.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>

#include <assert.h>

namespace gunit {
namespace tool {

namespace {

static class NullBuffer : public std::streambuf {
 public:
  int overflow(int c) { return c; }
} null_buffer;

static std::ostream null_stream(&null_buffer);

}  // namespace

void metadata_collector::handleNamespace(const cppast::cpp_entity& e,
                                         const bool enter) {
  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

void metadata_collector::handleClass(const cppast::cpp_class& e, bool enter) {
  if (enter) {
    assert(!m_current.has_value());
    m_current = ClassMetadata{};
  } else {
    assert(m_current.has_value());
    m_storage.emplace(m_namespaces.scope() + "::" + e.name(),
                      m_current.value());
    m_current.reset();
  }
}

void metadata_collector::handleConstructor(const cppast::cpp_constructor&) {
  assert(m_current.has_value());
  m_current->hasUserConstructors = true;
  m_current->hasAnyMethods = true;
}

void metadata_collector::handleMemberFunction(
    const cppast::cpp_member_function&) {
  assert(m_current.has_value());
  m_current->hasAnyMethods = true;
}

void metadata_collector::handleMemberVariable(
    const cppast::cpp_member_variable&) {
  assert(m_current.has_value());
  m_current->hasAnyFields = true;
}

}  // namespace tool
}  // namespace gunit
