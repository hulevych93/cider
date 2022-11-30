#pragma once

#include <string>
#include "recorder/details/action.h"

namespace gunit {
namespace recorder {
namespace lua {
std::string produceFunctionCall(const char* moduleName,
                                const char* name,
                                size_t paramCount,
                                bool keepResultAsLocalVar,
                                bool object = false);

std::string produceBinaryOpCall(BinaryOpType);

}  // namespace lua
}  // namespace recorder
}  // namespace gunit
