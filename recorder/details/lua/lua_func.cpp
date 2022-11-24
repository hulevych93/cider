#include "lua_func.h"

namespace gunit {
namespace recorder {

std::string produceFunctionCall(const char* moduleName,
                                const char* functionName,
                                size_t paramCount,
                                bool hasReturnValue,
                                bool object) {
  std::string funcTemplate;
  if (hasReturnValue) {
    funcTemplate = "local {} = ";
  }
  if (object) {
    funcTemplate += "{}:";
  } else {
    funcTemplate += moduleName;
    funcTemplate += ".";
  }
  funcTemplate += functionName;
  constexpr const char* ParamPlaceholer = "{}";
  funcTemplate += "(";
  for (auto i = 0u; i < paramCount; ++i) {
    if (i > 0u)
      funcTemplate += ", ";
    funcTemplate += ParamPlaceholer;
  }
  funcTemplate += ")";
  return funcTemplate;
}

std::string produceBinaryOpCall(const BinaryOpType type,
                                const bool hasReturnValue) {
  std::string funcTemplate;
  if (hasReturnValue) {
    funcTemplate = "local {} = ";
  }

  funcTemplate += "{}";
  switch (type) {
    case BinaryOpType::Assignment:
      funcTemplate += " = ";
      break;
  }

  funcTemplate += "{}";
  return funcTemplate;
}

}  // namespace recorder
}  // namespace gunit
