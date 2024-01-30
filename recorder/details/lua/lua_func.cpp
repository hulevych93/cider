#include "lua_func.h"

#include <unordered_map>

namespace cider {
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

/*%rename(equalOp) operator =;
%rename(plusEqualOp) operator +=;
%rename(minusEqualOp) operator -=;
%rename(multiplyEqualOp) operator *=;
%rename(divideEqualOp) operator /=;
%rename(percentEqualOp) operator %=;
%rename(plusOp) operator +;
%rename(minusOp) operator -;
%rename(multiplyOp) operator *;
%rename(divideOp) operator /;
%rename(percentOp) operator %;
%rename(notOp) operator !;
%rename(indexIntoConstOp) operator[](unsigned idx) const;
%rename(indexIntoOp) operator[](unsigned idx);
%rename(functorOp) operator ();
%rename(equalEqualOp) operator ==;
%rename(notEqualOp) operator !=;
%rename(lessThanOp) operator <;
%rename(lessThanEqualOp) operator <=;
%rename(greaterThanOp) operator >;
%rename(greaterThanEqualOp) operator >=;
%rename(andOp) operator &&;
%rename(orOp) operator ||;
%rename(plusPlusPrefixOp) operator++();
%rename(plusPlusPostfixOp) operator++(int);
%rename(minusMinusPrefixOp) operator--();
%rename(minusMinusPostfixOp) operator--(int);

%rename(toBool) *::operator bool;
%rename(toFloat) *::operator float;
%rename(toDouble) *::operator double;
%rename(toLongDouble) *::operator long double;
%rename(toChar) *::operator char;
%rename(toUnsignedChar) *::operator unsigned char;
%rename(toShort) *::operator short;
%rename(toInt) *::operator int;
%rename(toLong) *::operator long;
%rename(toLongLong) *::operator long long;
%rename(toUnsignedShort) *::operator unsigned short;
%rename(toUnsignedInt) *::operator unsigned int;
%rename(toUnsignedLong) *::operator unsigned long;
%rename(toUnsignedLongLong) *::operator unsigned long long;
%rename(toString) *::operator std::string;
%rename(toConstCharString) *::operator const char*;
*/

std::string mutateFunctionName(const char* name) {
    static const std::unordered_map<std::string, std::string> Mapping = {
        {"operator bool", "toBool"},
        {"operator float", "toFloat"}
    };
    const auto it = Mapping.find(name);
    if(it != Mapping.end()) {
        return it->second;
    }
    return name;
}

}  // namespace lua
}  // namespace recorder
}  // namespace cider
