#pragma once

#include <cstdint>
#include <string>

namespace cider {
namespace models {

// string as field
struct StringStruct final {
  std::string field;
};

std::string string_struct_as_arg(const StringStruct& arg);

std::uint16_t uint16_from_32(const std::uint32_t arg);

}  // namespace models
}  // namespace cider
