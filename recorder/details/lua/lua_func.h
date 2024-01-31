#pragma once

#include <string>
#include <optional>

#include "recorder/details/action.h"

namespace cider {
namespace recorder {
namespace lua {
std::string produceFunctionCall(const char* moduleName,
                                const char* functionName,
                                const bool localNeeded,
                                const bool isNew,
                                size_t paramCount,
                                bool object = false);

std::string produceBinaryOpCall(BinaryOpType);

std::string mutateFunctionName(const char* name);

}  // namespace lua
}  // namespace recorder
}  // namespace cider
