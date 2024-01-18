#pragma once

#include <cstdint>
#include <string>

namespace cider {
namespace models {

// string as field
struct StringStruct final {
  ::std::string field;
  std::uint32_t val;
};

std::basic_string<char> string_struct_as_arg(const StringStruct& arg);

}  // namespace models
}  // namespace cider
