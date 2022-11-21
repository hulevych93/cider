#include "params.h"

#include "user_data.h"
#include "utils.h"

#include <sstream>

namespace gunit {
namespace recorder {

bool Nil::operator==(const Nil&) const {
  return true;
}

Argument ParamVisitor::operator()(const Nil&) const {
  return Argument{"nil"};
}

Argument ParamVisitor::operator()(const bool value) const {
  return value ? "true" : "false";
}

Argument ParamVisitor::operator()(const float value) const {
  // Independent from "C" locale conversion approach.
  // The decimal delimeter sign is always a dot.

  std::stringstream stream;
  stream.imbue(std::locale::classic());
  stream << value;
  return stream.str();
}

Argument ParamVisitor::operator()(const char* param) const {
  return (*this)(std::string{param});
}

Argument ParamVisitor::operator()(const std::string& value) const {
  return std::string{"'" + escape(value) + "'"};
}

UserDataParamVisitor::UserDataParamVisitor(CodeSink& sink) : _sink(sink) {}

Argument UserDataParamVisitor::operator()(const UserDataParamPtr& value) const {
  return (*value).generate(_sink);
}

}  // namespace recorder
}  // namespace gunit
