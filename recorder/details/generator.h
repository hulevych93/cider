#pragma once

#include "action.h"

#include <memory>
#include <string>

namespace gunit {
namespace recorder {

struct ScriptGenerationError final : public std::exception {
  explicit ScriptGenerationError(const char* msg);

  const char* what() const _NOEXCEPT override { return _error.c_str(); }

 private:
  std::string _error;
};

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

  std::string operator()(const UserDataParamPtr& value) const;

 private:
  CodeSink& _sink;
};

//` The `ScriptGenerator` class is intended for generating Lua script.
//` It processes the user action log entries and generates script source code.
class ScriptGenerator final {
 public:
  void operator()(const FreeFunctionCall& context);

  //` The method returns ready to use script. This method resets the state of
  // the generator and ` it becomes ready to further usage. ` Throws:
  //`ScriptGenerationError`.
  std::string getScript();

 private:
  std::string _body;
};

}  // namespace recorder
}  // namespace gunit
