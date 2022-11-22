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

//` The `ScriptGenerator` class is intended for generating Lua script.
//` It processes the user action log entries and generates script source code.
class ScriptGenerator final {
 public:
  ScriptGenerator(ParamCodeProducer);

  void operator()(const FreeFunctionCall& context);

  //` The method returns ready to use script. This method resets the state of
  // the generator and ` it becomes ready to further usage. ` Throws:
  //`ScriptGenerationError`.
  std::string getScript();

 private:
  ParamCodeProducer _paramProducer;
  std::string _body;
};

}  // namespace recorder
}  // namespace gunit
