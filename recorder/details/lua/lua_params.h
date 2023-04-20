#pragma once

#include "recorder/details/params.h"

#include <memory>
#include <string>

namespace cider {
namespace recorder {

class CodeSink;

namespace lua {

//` The `ParamVisitor` produces code chunks to be used as arguments during
//` code generation.
struct ParamVisitor {
  template <
      typename Type,
      std::enable_if_t<std::is_integral_v<Type> &&
                       !std::is_same_v<std::decay_t<Type>, bool>>* = nullptr>
  std::string operator()(Type param) {
    return std::to_string(param);
  }

  std::string operator()(const Nil&) const;
  std::string operator()(bool value) const;
  std::string operator()(float value) const;
  std::string operator()(const char* value) const;
  std::string operator()(const std::string& value) const;
};

//` The `UserDataParamVisitor` is extended with UserDataParamPtr operator
// overload.
class UserDataParamVisitor final : public ParamVisitor {
 public:
  explicit UserDataParamVisitor(CodeSink& sink);

  using ParamVisitor::operator();

  std::string operator()(const UserDataValueParamPtr& value) const;

 private:
  CodeSink& _sink;
};

std::string produceParamCode(const Param& param, CodeSink& sink);

}  // namespace lua
}  // namespace recorder
}  // namespace cider
