#include "lua_func.h"

#include <unordered_map>

namespace cider {
namespace recorder {
namespace lua {

std::string produceFunctionCall(const char* moduleName,
                                const char* functionName,
                                const bool localNeeded,
                                const bool isNew,
                                size_t paramCount,
                                bool object) {
  std::string funcTemplate;
  if (localNeeded) {
    if (isNew) {
      funcTemplate += "local ";
    }
    funcTemplate += "{} = ";
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

std::string produceUnaryOpCall(const bool localNeeded,
                               const bool isNew,
                               const UnaryOpType type) {
  std::string funcTemplate;
  if (localNeeded) {
      if (isNew) {
          funcTemplate += "local ";
      }
      funcTemplate += "{} = ";
  }
  if(type == UnaryOpType::Minus) {
      funcTemplate += "-{}";
  }
  return funcTemplate;
}

std::string mutateFunctionName(const char* name) {
  static const std::unordered_map<std::string, std::string> Mapping = {
      {"operator bool", "toBool"},
      {"operator float", "toFloat"},
      {"operator double", "toDouble"},
      {"operator long double", "toLongDouble"},
      {"operator char", "toChar"},
      {"operator short", "toShort"},
      {"operator int", "toInt"},
      {"operator long", "toLong"},
      {"operator long long", "toLongLong"},
      {"operator unsigned char", "toUnsignedChar"},
      {"operator unsigned short", "toUnsignedShort"},
      {"operator unsigned int", "toUnsignedInt"},
      {"operator unsigned long", "toUnsignedLong"},
      {"operator unsigned long long", "toUnsignedLongLong"},
      {"operator string", "toString"},
      {"operator basic_string", "toString"},
      {"operator const char *", "toConstCharString"},
      {"operator=", "assignOp"},
      {"operator<", "lessThanOp"},
      {"operator+=", "plusEqualOp"},
      {"operator<=", "lessThanEqualOp"},
      {"operator-=", "minusEqualOp"},
      {"operator>", "greaterThanOp"},
      {"operator*=", "multiplyEqualOp"},
      {"operator>=", "greaterThanEqualOp"},
      {"operator/=", "divideEqualOp"},
      {"operator&&", "andOp"},
      {"operator%=", "percentEqualOp"},
      {"operator||", "orOp"},
      {"operator+", "plusOp"},
      {"operator++", "plusPlusPrefixOp"},
      {"operator-", "minusOp"},
      {"operator--", "minusMinusPrefixOp"},
      {"operator/", "divideOp"},
      {"operator*", "multiplyOp"},
      {"operator%", "percentOp"},
      {"operator!", "notOp"},
      {"operator[]", "indexIntoOp"},
      {"operator()", "functorOp"},
      {"operator==", "equalEqualOp"},
      {"operator!=", "notEqualOp"},
  };
  const auto it = Mapping.find(name);
  if (it != Mapping.end()) {
    return it->second;
  }
  return name;
}

}  // namespace lua
}  // namespace recorder
}  // namespace cider
