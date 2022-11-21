#include "utils.h"

namespace gunit {
namespace recorder {

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

}  // namespace recorder
}  // namespace gunit
