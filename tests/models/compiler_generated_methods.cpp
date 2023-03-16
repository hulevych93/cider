#include "compiler_generated_methods.h"

namespace gunit {
namespace models {

void NoConstructors::doo() {
  ++m_data;
}

OnlyCopyConstructor::~OnlyCopyConstructor() {}

void OnlyCopyConstructor::doo() {
  ++m_data;
}

}  // namespace models
}  // namespace gunit
