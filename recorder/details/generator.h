// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/recorder.h"

#include <memory>
#include <string>

namespace cider {
namespace recorder {

using ParamCodeProducer = std::string (*)(const std::string& name,
                                          const Param& param,
                                          CodeSink& sink);
using FunctionCodeProducer = std::string (*)(const char* moduleName,
                                             const char* functionName,
                                             const bool localNeeded,
                                             const bool isNew,
                                             size_t paramCount,
                                             bool object);
using UnaryOpCodeProducer = std::string (*)(const bool,
                                            const bool,
                                            const UnaryOpType);
using BinaryOpCodeProducer = std::string (*)(BinaryOpType);
using FunctionNameMutator = std::string (*)(const char* name);

struct LanguageContext final {
  ParamCodeProducer paramProducer = nullptr;
  FunctionCodeProducer funcProducer = nullptr;
  UnaryOpCodeProducer unaryOpProducer = nullptr;
  BinaryOpCodeProducer binaryOpProducer = nullptr;
  FunctionNameMutator functionNameMutator = nullptr;
};

class ScriptGenerator final {
 public:
  ScriptGenerator(std::string moduleName, LanguageContext);
  ScriptGenerator(ScriptGenerator&&) = default;
  ~ScriptGenerator();

  void operator()(const Function& context);
  void operator()(const ClassMethod& context);
  void operator()(const ClassBinaryOp& context);
  void operator()(const ClassUnaryOp& context);
  void operator()(const ClassDestructor& context);

  std::string getScript();
  void discard();

 private:
  std::vector<std::string> produceArgs(const Params& params) const;

  LocalVar processResult(const Param& param);

 private:
  std::string _module;
  LanguageContext _langContext;

  std::unique_ptr<CodeSink> _sink;
};

ScriptGenerator makeLuaGenerator(const std::string& moduleName);

std::string generateScript(ScriptGenerator& generator,
                           const std::vector<cider::recorder::Action>& actions,
                           const size_t limit);

}  // namespace recorder
}  // namespace cider
