#include "lua_params.h"

#include <sstream>

namespace gunit {
namespace recorder {

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

std::string ParamVisitor::operator()(const Nil&) const {
  return std::string{"nil"};
}

std::string ParamVisitor::operator()(const bool value) const {
  return value ? "true" : "false";
}

std::string ParamVisitor::operator()(const float value) const {
  // Independent from "C" locale conversion approach.
  // The decimal delimeter sign is always a dot.

  std::stringstream stream;
  stream.imbue(std::locale::classic());
  stream << value;
  return stream.str();
}

std::string ParamVisitor::operator()(const char* param) const {
  return (*this)(std::string{param});
}

std::string ParamVisitor::operator()(const std::string& value) const {
  return std::string{"'" + escape(value) + "'"};
}

UserDataParamVisitor::UserDataParamVisitor(CodeSink& sink) : _sink(sink) {}

std::string UserDataParamVisitor::operator()(
    const UserDataParamPtr& value) const {
  return (*value).generateCode(_sink);
}

std::string produceLuaCode(const Param& param, CodeSink& sink) {
  return std::visit(UserDataParamVisitor{sink}, param);
}

}  // namespace recorder
}  // namespace gunit
