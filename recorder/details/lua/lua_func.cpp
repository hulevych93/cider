#include "lua_func.h"

namespace gunit {
namespace recorder {
namespace lua {

std::string produceFunctionCall(const char* moduleName,
                                const char* name,
                                size_t paramCount,
                                const bool keepResultAsLocalVar,
                                const bool object) {
  std::string funcTemplate;
  if (keepResultAsLocalVar) {
    funcTemplate = "local {} = ";
  }
  if (object) {
    funcTemplate += "{}:";
  } else {
    funcTemplate += moduleName;
    funcTemplate += ".";
  }
  funcTemplate += name;
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

std::string produceBinaryOpCall(const BinaryOpType type) {
  std::string funcTemplate = "{}";
  switch (type) {
    case BinaryOpType::Assignment:
      funcTemplate += " = ";
      break;
  }

  funcTemplate += "{}";
  return funcTemplate;
}

}  // namespace lua
}  // namespace recorder
}  // namespace gunit
