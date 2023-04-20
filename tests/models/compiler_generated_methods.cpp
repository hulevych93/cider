#include "compiler_generated_methods.h"

namespace cider {
namespace models {

void NoConstructors::doo() {
  ++m_data;
}

OnlyDestructor::~OnlyDestructor() {}

void OnlyDestructor::doo() {
  ++m_data;
}

UserConstructor::UserConstructor(int data) : m_data(data) {}

void UserConstructor::doo() {
  ++m_data;
}

DefaultAndMoveConstructor::DefaultAndMoveConstructor(
    DefaultAndMoveConstructor&& that) {}

DefaultAndMoveConstructor::DefaultAndMoveConstructor() = default;

void DefaultAndMoveConstructor::doo() {
  ++m_data;
}

DefaultAndCopyConstructor::DefaultAndCopyConstructor(
    const DefaultAndCopyConstructor& that) {}

DefaultAndCopyConstructor::DefaultAndCopyConstructor() = default;

void DefaultAndCopyConstructor::doo() {
  ++m_data;
}

OnlyCopyOperator& OnlyCopyOperator::operator=(const OnlyCopyOperator&) {
  return *this;
}

void OnlyCopyOperator::doo() {}

OnlyMoveOperator& OnlyMoveOperator::operator=(OnlyMoveOperator&&) {
  return *this;
}

void OnlyMoveOperator::doo() {}

}  // namespace models
}  // namespace cider
