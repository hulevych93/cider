#include "lua_params.h"

#include <sstream>

namespace cider {
namespace recorder {
namespace lua {

namespace {

std::string escape(const std::string& str) {
  std::string output;

  for (auto it = str.begin(); it != str.end();) {
    const auto codepoint = *it;
    ++it;
    switch (codepoint) {
      case '\'':
        output += "\\'";
        break;
      case '\n':
        output += "\\n";
        break;
      case '\r':
        output += "\\r";
        break;
      case '\t':
        output += "\\t";
        break;
      case '"':
        output += "\\\"";
        break;
      case '\\':
        output += "\\\\";
        break;
      case '\a':
        output += "\\a";
        break;
      case '\b':
        output += "\\b";
        break;
      case '\v':
        output += "\\v";
        break;
      case '\f':
        output += "\\f";
        break;
      case '\e':
        output += "\\e";
        break;
      default:
        output += codepoint;
    }
  }

  return output;
}

}  // namespace

ParamVisitor::ParamVisitor(const std::string& moduleName)
    : _moduleName(moduleName) {}

std::string ParamVisitor::operator()(const Nil&) const {
  return std::string{"nil"};
}

std::string ParamVisitor::operator()(const bool value) const {
  return value ? "true" : "false";
}

std::string ParamVisitor::operator()(const double value) const {
  // Independent from "C" locale conversion approach.
  // The decimal delimeter sign is always a dot.

  std::stringstream stream;
  stream.imbue(std::locale::classic());
  stream << (value);
  return stream.str();
}

std::string ParamVisitor::operator()(const IntegerType& value) const {
  return std::visit([this](const auto& val) { return (*this)(val); }, value);
}

std::string ParamVisitor::operator()(char value) const {
  return _moduleName + ".Char(" + std::to_string(value) + ")";
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(short value) const {
  return _moduleName + ".Short(" + std::to_string(value) + ")";
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(int value) const {
  return _moduleName + ".Int(" + std::to_string(value) + ")";
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(long value) const {
  return _moduleName + ".Long(" + std::to_string(value) + ")";
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(long long value) const {
  return _moduleName + ".LongLong(" + std::to_string(value) + ")";
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(unsigned char value) const {
  return _moduleName + ".UChar(" + std::to_string(value) + ")";
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(unsigned short value) const {
  return _moduleName + ".UShort(" + std::to_string(value) + ")";
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(unsigned int value) const {
  return _moduleName + ".UInt(" + std::to_string(value) + ")";
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(unsigned long value) const {
  return _moduleName + ".ULong(" + std::to_string(value) + ")";
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(unsigned long long value) const {
  return _moduleName + ".ULongLong(" + std::to_string(value) + ")";
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(const char* param) const {
  return (*this)(std::string{param});
}  // LCOV_EXCL_LINE

std::string ParamVisitor::operator()(const std::string& value) const {
  return std::string{"'" + escape(value) + "'"};
}  // LCOV_EXCL_LINE

UserDataParamVisitor::UserDataParamVisitor(const std::string& moduleName,
                                           CodeSink& sink)
    : ParamVisitor(moduleName), _sink(sink) {}

std::string UserDataParamVisitor::operator()(
    const UserDataValueParamPtr& value) const {
  return (*value).generateCode(_moduleName, _sink);
}

std::string produceParamCode(const std::string& moduleName,
                             const Param& param,
                             CodeSink& sink) {
  return std::visit(UserDataParamVisitor{moduleName, sink}, param);
}

}  // namespace lua
}  // namespace recorder
}  // namespace cider
