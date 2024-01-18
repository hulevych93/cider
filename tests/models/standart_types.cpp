#include "standart_types.h"

namespace cider {
namespace models {

std::string string_struct_as_arg(const StringStruct& arg) {
  return arg.field;
}

}  // namespace models
}  // namespace cider
