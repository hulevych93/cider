#pragma once

#include "action.h"

#include <memory>
#include <string>

namespace gunit {
namespace recorder {

std::string getFreeFunctionCallTemplate();

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
  ~ScriptGenerator();

  void operator()(const FreeFunctionCall& context);
  void operator()(const ClassConstructorCall& context);
  void operator()(const ClassMethodCall& context);

  //` The method returns ready to use script. This method resets the state of
  // the generator and ` it becomes ready to further usage. ` Throws:
  //`ScriptGenerationError`.
  std::string getScript();

 private:
  std::vector<std::string> produceArgs(const Params& params) const;

 private:
  ParamCodeProducer _paramProducer;

  std::unique_ptr<CodeSink> _sink;
};

}  // namespace recorder
}  // namespace gunit
