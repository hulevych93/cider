#pragma once

#include "recorder/details/action.h"
#include "recorder/recorder.h"

#include <memory>
#include <string>

namespace cider {
namespace recorder {

using ParamCodeProducer = std::string (*)(const Param& param, CodeSink& sink);
using FunctionCodeProducer = std::string (*)(const char* moduleName,
                                             const char* name,
                                             size_t paramCount,
                                             bool hasReturnValue,
                                             bool object);
using BinaryOpCodeProducer = std::string (*)(BinaryOpType);

struct LanguageContext final {
  ParamCodeProducer paramProducer = nullptr;
  FunctionCodeProducer funcProducer = nullptr;
  BinaryOpCodeProducer binaryOpProducer = nullptr;
};

//` The `ScriptGenerator` class is intended for generating Lua script.
//` It processes the user action log entries and generates script source code.
class ScriptGenerator final {
 public:
  ScriptGenerator(std::string moduleName, LanguageContext);
  ScriptGenerator(ScriptGenerator&&) = default;
  ~ScriptGenerator();

  void operator()(const Function& context);
  void operator()(const ClassMethod& context);
  void operator()(const ClassBinaryOp& context);

  //` The method returns ready to use script. This method resets the state of
  // the generator and ` it becomes ready to further usage. ` Throws:
  //`ScriptGenerationError`.
  std::string getScript();
  void discard();

 private:
  std::vector<std::string> produceArgs(const Params& params) const;
  std::string processResult(const Param& param);

 private:
  std::string _module;
  LanguageContext _langContext;

  std::unique_ptr<CodeSink> _sink;
};

}  // namespace recorder
}  // namespace cider
