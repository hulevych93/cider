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

}  // namespace models
}  // namespace gunit
