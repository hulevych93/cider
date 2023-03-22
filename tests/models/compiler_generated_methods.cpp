#include "compiler_generated_methods.h"

namespace gunit {
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

DefaultAndMoveConstructor::DefaultAndMoveConstructor(DefaultAndMoveConstructor&& that) {

}

DefaultAndMoveConstructor::DefaultAndMoveConstructor() = default;

void DefaultAndMoveConstructor::doo() {
    ++m_data;
}

}  // namespace models
}  // namespace gunit
