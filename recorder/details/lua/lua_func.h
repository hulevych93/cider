#pragma once

#include <string>
#include "recorder/details/action.h"

namespace gunit {
namespace recorder {

std::string produceFunctionCall(const char* moduleName,
                                const char* functionName,
                                size_t paramCount,
                                bool hasReturnValue,
                                bool object = false);

std::string produceBinaryOpCall(BinaryOpType, bool hasReturnValue);

}  // namespace recorder
}  // namespace gunit
