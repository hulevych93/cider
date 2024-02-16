// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "generator.h"
#include "params.h"

#include <fmt/args.h>
#include <fmt/format.h>

#include "recorder/details/lua/lua_func.h"
#include "recorder/details/lua/lua_params.h"

#include <unordered_map>

namespace cider {
namespace recorder {

namespace {

class CodeSinkImpl : public CodeSink {
 public:
  std::string getScript() {
    _locals.clear();
    _localCounter = 0u;
    _lineCounter = 0u;
    return std::move(_sink);
  }

 private:
  std::string processLocalVar(const std::string& codeTemplate) override {
    const auto actualLocalName = suggestLocalName();

    fmt::dynamic_format_arg_store<fmt::format_context> fmt_args;
    fmt_args.push_back(fmt::arg("var", actualLocalName));

    format(codeTemplate, fmt_args);

    return actualLocalName;
  }

  void processFunctionCall(const void* object,
                           const std::string& localName,
                           const std::vector<std::string>& args,
                           const std::string& codeTemplate) override {
    fmt::dynamic_format_arg_store<fmt::format_context> fmt_args;

    if (!localName.empty()) {
      fmt_args.push_back(localName);
    }

    if (object) {
      fmt_args.push_back(getObjectLocal(object));
    }

    for (const auto& arg : args) {
      fmt_args.push_back(arg);
    }

    format(codeTemplate, fmt_args);
  }

  std::string searchForLocalVar(const void* object) const override {
    return getObjectLocal(object);
  }

  LocalVar registerLocalVar(const void* object) override {
    LocalVar var;
    var.isNew = IsAbsent(object);
    var.name = suggestLocalName(object);
    return var;
  }

  void unregisterLocalVar(const void* object) override {
    const auto localIt = _locals.find(object);
    if (localIt != _locals.end()) {
      _sink += localIt->second + " = nil\n";  // TODO
      _locals.erase(localIt);
    }
  }

 private:
  std::string suggestLocalName(const void* object = nullptr) {
    if (object) {
      auto it = _locals.find(object);
      if (it == _locals.end()) {
        const auto suggested =
            std::string{"object"} + "_" + std::to_string(++_localCounter);
        _locals.emplace(object, suggested);

        _sink += "collectgarbage('collect')\n";

        return suggested;
      } else {
        return it->second;
      }
    }

    return std::string{"object"} + "_" + std::to_string(++_localCounter);
  }

  bool IsAbsent(const void* object) const {
    const auto it = _locals.find(object);
    return it == _locals.end();
  }

  std::string getObjectLocal(const void* object) const {
    const auto localIt = _locals.find(object);
    if (localIt != _locals.end()) {
      return localIt->second;
    }

    return "NAN";
  }  // LCOV_EXCL_LINE

  void format(const std::string& codeTemplate,
              const fmt::dynamic_format_arg_store<fmt::format_context>& args) {
    _sink + "print(" + std::to_string(_lineCounter) + ") ";
    try {
      _sink += fmt::vformat(codeTemplate, args);
    } catch (const fmt::format_error& fmtErr) {
      throw ScriptGenerationError{
          fmt::format(
              "Code template \'{}\' generation is failed. Details: \'{}\'",
              codeTemplate, fmtErr.what())
              .c_str()};
    }

    if (_sink.back() != '\n') {
      _sink += "\n";
    }
  }

 private:
  std::string _sink;
  std::unordered_map<const void*, std::string> _locals;
  size_t _localCounter = 0u;
  size_t _lineCounter = 0u;
};

std::string nullParamProcessor(const std::string&, const Param&, CodeSink&) {
  throw ScriptGenerationError{
      "nullParamProcessor for script generator is set."};
}  // LCOV_EXCL_LINE

std::string nullFuncProcessor(const char*,
                              const char*,
                              const bool,
                              const bool,
                              size_t,
                              bool) {
  throw ScriptGenerationError{"nullFuncProcessor for script generator is set."};
}  // LCOV_EXCL_LINE

std::string nullBinaryOpProcessor(BinaryOpType) {
  throw ScriptGenerationError{
      "nullBinaryOpProcessor for script generator is set."};
}  // LCOV_EXCL_LINE

std::string nullUnaryOpProcessor(bool, bool, UnaryOpType) {
  throw ScriptGenerationError{
      "nullUnaryOpProcessor for script generator is set."};
}  // LCOV_EXCL_LINE

LanguageContext fixLanguageContext(LanguageContext context) {
  if (!context.funcProducer) {
    context.funcProducer = nullFuncProcessor;
  }
  if (!context.paramProducer) {
    context.paramProducer = nullParamProcessor;
  }
  if (!context.binaryOpProducer) {
    context.binaryOpProducer = nullBinaryOpProcessor;
  }
  if (!context.unaryOpProducer) {
    context.unaryOpProducer = nullUnaryOpProcessor;
  }
  return context;
}

}  // namespace

ScriptGenerationError::ScriptGenerationError(const char* msg)
    : _error(fmt::format("ScriptGenerationError: {}", msg)) {}

ScriptGenerator::ScriptGenerator(std::string moduleName,
                                 const LanguageContext context)
    : _module(std::move(moduleName)),
      _langContext(fixLanguageContext(context)),
      _sink(std::make_unique<CodeSinkImpl>()) {}

ScriptGenerator::~ScriptGenerator() = default;

void ScriptGenerator::operator()(const Function& context) {
  const auto result = processResult(context.retVal);
  const auto mutatedName = _langContext.functionNameMutator(context.name);

  const auto codeTemplate = _langContext.funcProducer(
      _module.c_str(), mutatedName.c_str(), !result.name.empty(), result.isNew,
      context.params.size(), false);

  const auto args = produceArgs(context.params);
  _sink->processFunctionCall(nullptr, result.name, args, codeTemplate);
}

void ScriptGenerator::operator()(const ClassMethod& context) {
  const auto result = processResult(context.method.retVal);
  const auto mutatedName =
      _langContext.functionNameMutator(context.method.name);

  const auto codeTemplate = _langContext.funcProducer(
      _module.c_str(), mutatedName.c_str(), !result.name.empty(), result.isNew,
      context.method.params.size(), true);

  const auto args = produceArgs(context.method.params);
  _sink->processFunctionCall(context.objectAddress, result.name, args,
                             codeTemplate);
}

void ScriptGenerator::operator()(const ClassBinaryOp& context) {
  const auto codeTemplate = _langContext.binaryOpProducer(context.opName);

  const auto args = produceArgs({context.param});
  _sink->processFunctionCall(context.objectAddress, std::string{}, args,
                             codeTemplate);
}

void ScriptGenerator::operator()(const ClassUnaryOp& context) {
  const auto result = processResult(context.retVal);

  const auto codeTemplate = _langContext.unaryOpProducer(
      !result.name.empty(), result.isNew, context.opName);

  _sink->processFunctionCall(context.objectAddress, result.name, {},
                             codeTemplate);
}

void ScriptGenerator::operator()(const ClassDestructor& context) {
  _sink->unregisterLocalVar(context.objectAddress);
}

std::vector<std::string> ScriptGenerator::produceArgs(
    const Params& params) const {
  std::vector<std::string> args;
  args.reserve(params.size());
  for (const auto& param : params) {
    args.emplace_back(_langContext.paramProducer(_module, param, *_sink));
  }
  return args;
}

LocalVar ScriptGenerator::processResult(const Param& param) {
  if (auto* userData = std::get_if<UserDataReferenceParamPtr>(&param)) {
    return (*userData)->registerLocal(*_sink);
  }
  return {};
}

std::string ScriptGenerator::getScript() {
  return static_cast<CodeSinkImpl&>(*_sink).getScript();
}

void ScriptGenerator::discard() {
  static_cast<CodeSinkImpl&>(*_sink).getScript();
}

ScriptGenerator makeLuaGenerator(const std::string& moduleName) {
  LanguageContext context;
  context.funcProducer = lua::produceFunctionCall;
  context.paramProducer = lua::produceParamCode;
  context.binaryOpProducer = lua::produceBinaryOpCall;
  context.functionNameMutator = lua::mutateFunctionName;
  context.unaryOpProducer = lua::produceUnaryOpCall;
  return ScriptGenerator{moduleName, context};
}

}  // namespace recorder
}  // namespace cider
