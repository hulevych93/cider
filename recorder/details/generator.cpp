#include "generator.h"
#include "params.h"

#include <fmt/args.h>
#include <fmt/format.h>

#include <unordered_map>

namespace gunit {
namespace recorder {

namespace {

class CodeSinkImpl : public CodeSink {
 public:
  std::string getScript() {
    _locals.clear();
    _localCounter = 0u;
    return std::move(_sink);
  }

 private:
  std::string processLocalVar(const char* localName,
                              const std::string& codeTemplate) override {
    const auto actualLocalName = suggestLocalName(localName);

    fmt::dynamic_format_arg_store<fmt::format_context> fmt_args;
    fmt_args.push_back(fmt::arg(localName, actualLocalName));

    format(codeTemplate, fmt_args);

    return actualLocalName;
  }

  std::string processFunctionCall(const char* localName,
                                  const std::vector<std::string>& args,
                                  const std::string& codeTemplate) override {
    return functionCall(nullptr, false, localName, args, codeTemplate);
  }

  std::string processConstructorCall(const void* object,
                                     const char* localName,
                                     const std::vector<std::string>& args,
                                     const std::string& codeTemplate) override {
    return functionCall(object, false, localName, args, codeTemplate);
  }

  std::string processMethodCall(const void* object,
                                const char* localName,
                                const std::vector<std::string>& args,
                                const std::string& codeTemplate) override {
    return functionCall(object, true, localName, args, codeTemplate);
  }

 private:
  std::string searchForLocalVar(const void* object) const override {
    return getObjectLocal((void*)object);
  }

  std::string suggestLocalName(const char* localName,
                               const void* object = nullptr) {
    assert(localName);
    const auto suggested =
        std::string{localName} + std::to_string(++_localCounter);
    if (object) {
      _locals.emplace(object, suggested);
    }
    return suggested;
  }

  std::string getObjectLocal(const void* object) const {
    const auto localIt = _locals.find(object);
    if (localIt != _locals.end()) {
      return localIt->second;
    }

    throw std::logic_error{"Object is unreachable."};
  }

  std::string functionCall(const void* object,
                           bool isMethodCall,
                           const char* localName,
                           const std::vector<std::string>& args,
                           const std::string& codeTemplate) {
    std::string actualLocalName;

    fmt::dynamic_format_arg_store<fmt::format_context> fmt_args;

    if (localName) {
      // TODO: fix me
      actualLocalName = suggestLocalName(localName, object);
      fmt_args.push_back(actualLocalName);
    }

    if (isMethodCall && object) {
      fmt_args.push_back(getObjectLocal(object));
    }

    for (const auto& arg : args) {
      fmt_args.push_back(arg);
    }

    format(codeTemplate, fmt_args);

    return actualLocalName;
  }

  void format(const std::string& codeTemplate,
              const fmt::dynamic_format_arg_store<fmt::format_context>& args) {
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
};

}  // namespace

ScriptGenerationError::ScriptGenerationError(const char* msg)
    : _error(fmt::format("ScriptGenerationError: {}", msg)) {}

ScriptGenerator::ScriptGenerator(std::string moduleName,
                                 const LanguageContext context)
    : _module(std::move(moduleName)),
      _langContext(context),
      _sink(std::make_unique<CodeSinkImpl>()) {}

ScriptGenerator::~ScriptGenerator() = default;

void ScriptGenerator::operator()(const FreeFunction& context) {
  const auto codeTemplate = _langContext.funcProducer(
      _module.c_str(), context.functionName, context.params.size(),
      context.hasReturnValue, false);

  const auto args = produceArgs(context.params);

  if (context.hasReturnValue) {
    // TODO: fix me
    _sink->processFunctionCall("nullptr", args, codeTemplate);
  } else {
    _sink->processFunctionCall(nullptr, args, codeTemplate);
  }
}

void ScriptGenerator::operator()(const ClassConstructor& context) {
  const auto codeTemplate = _langContext.funcProducer(
      _module.c_str(), context.className, context.params.size(), true, false);

  const auto args = produceArgs(context.params);

  _sink->processConstructorCall(context.objectAddress, context.className, args,
                                codeTemplate);
}

void ScriptGenerator::operator()(const ClassMethod& context) {
  const auto codeTemplate = _langContext.funcProducer(
      _module.c_str(), context.methodName, context.params.size(),
      context.hasReturnValue, true);

  const auto args = produceArgs(context.params);

  _sink->processMethodCall(context.objectAddress, nullptr, args, codeTemplate);
}

void ScriptGenerator::operator()(const ClassBinaryOp& context) {
  const auto codeTemplate =
      _langContext.binaryOpProducer(context.opName, context.hasReturnValue);

  const auto args = produceArgs({context.param});

  _sink->processMethodCall(context.objectAddress, nullptr, args, codeTemplate);
}

std::vector<std::string> ScriptGenerator::produceArgs(
    const Params& params) const {
  std::vector<std::string> args;
  args.reserve(params.size());
  for (const auto& param : params) {
    args.emplace_back(_langContext.paramProducer(param, *_sink));
  }
  return args;
}

std::string ScriptGenerator::getScript() {
  return static_cast<CodeSinkImpl&>(*_sink).getScript();
}

}  // namespace recorder
}  // namespace gunit
