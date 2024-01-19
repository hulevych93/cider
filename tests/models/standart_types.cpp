#include "standart_types.h"

namespace cider {
namespace models {

std::string string_struct_as_arg(const StringStruct& arg) {
  return arg.field;
}

uint16_t uint16_from_32(const uint32_t arg) {
  return (uint16_t)arg;
}

}  // namespace models
}  // namespace cider
