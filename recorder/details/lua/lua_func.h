#pragma once

#include <string>

namespace gunit {
namespace recorder {

std::string produceFunctionCall(const char* moduleName,
                                const char* functionName,
                                size_t paramCount,
                                bool hasReturnValue,
                                bool object = false);

}  // namespace recorder
}  // namespace gunit
