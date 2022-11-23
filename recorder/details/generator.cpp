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

  std::string processConstructorCall(void* object,
                                     const char* localName,
                                     const std::vector<std::string>& args,
                                     const std::string& codeTemplate) override {
    return functionCall(object, false, localName, args, codeTemplate);
  }

  std::string processMethodCall(void* object,
                                const char* localName,
                                const std::vector<std::string>& args,
                                const std::string& codeTemplate) override {
    return functionCall(object, true, localName, args, codeTemplate);
  }

 private:
  std::string suggestLocalName(const char* localName, void* object = nullptr) {
    assert(localName);
    const auto suggested =
        std::string{localName} + std::to_string(++_localCounter);
    if (object) {
      _locals.emplace(object, suggested);
    }
    return suggested;
  }

  std::string getObjectLocal(void* object) const {
    const auto localIt = _locals.find(object);
    if (localIt != _locals.end()) {
      return localIt->second;
    }

    return {};
  }

  std::string functionCall(void* object,
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
  std::unordered_map<void*, std::string> _locals;
  size_t _localCounter = 0u;
};

struct ActionTemplateProducer final {
  std::string operator()(const FreeFunctionCall& functionCall) const {
    return produceFunctionCall(functionCall.functionName,
                               functionCall.params.size(),
                               functionCall.hasReturnValue);
  }

  std::string operator()(const ClassConstructorCall& constuctorCall) const {
    return produceFunctionCall(constuctorCall.className,
                               constuctorCall.params.size(), true);
  }

  std::string operator()(const ClassMethodCall& classMethodCall) const {
    return produceFunctionCall(classMethodCall.methodName,
                               classMethodCall.params.size(),
                               classMethodCall.hasReturnValue, true);
  }

 private:
  std::string produceFunctionCall(const char* functionName,
                                  size_t paramCount,
                                  bool hasReturnValue,
                                  bool object = false) const {
    std::string funcTemplate;
    if (hasReturnValue) {
      funcTemplate = "local {} = ";
    }
    if (object) {
      funcTemplate += "{}:";
    } else {
      funcTemplate += getFreeFunctionCallTemplate();
    }
    funcTemplate += functionName;
    constexpr const char* ParamPlaceholer = "{}";
    funcTemplate += "(";
    for (auto i = 0u; i < paramCount; ++i) {
      if (i > 0u)
        funcTemplate += ", ";
      funcTemplate += ParamPlaceholer;
    }
    funcTemplate += ")";
    return funcTemplate;
  }
};

std::string produceActionTemplate(const Action& action) {
  ActionTemplateProducer producer;
  return std::visit(producer, action);
}

}  // namespace

ScriptGenerationError::ScriptGenerationError(const char* msg)
    : _error(fmt::format("ScriptGenerationError: {}", msg)) {}

ScriptGenerator::ScriptGenerator(const ParamCodeProducer producer)
    : _paramProducer(producer), _sink(std::make_unique<CodeSinkImpl>()) {}

ScriptGenerator::~ScriptGenerator() = default;

void ScriptGenerator::operator()(const FreeFunctionCall& context) {
  const auto codeTemplate = produceActionTemplate(context);
  const auto args = produceArgs(context.params);

  if (context.hasReturnValue) {
    // TODO: fix me
    _sink->processFunctionCall("nullptr", args, codeTemplate);
  } else {
    _sink->processFunctionCall(nullptr, args, codeTemplate);
  }
}

void ScriptGenerator::operator()(const ClassConstructorCall& context) {
  const auto codeTemplate = produceActionTemplate(context);
  const auto args = produceArgs(context.params);

  _sink->processConstructorCall(context.objectAddress, context.className, args,
                                codeTemplate);
}

void ScriptGenerator::operator()(const ClassMethodCall& context) {
  const auto codeTemplate = produceActionTemplate(context);
  const auto args = produceArgs(context.params);

  _sink->processMethodCall(context.objectAddress, nullptr, args, codeTemplate);
}

std::vector<std::string> ScriptGenerator::produceArgs(
    const Params& params) const {
  std::vector<std::string> args;
  args.reserve(params.size());
  for (const auto& param : params) {
    args.emplace_back(_paramProducer(param, *_sink));
  }
  return args;
}

std::string ScriptGenerator::getScript() {
  return static_cast<CodeSinkImpl&>(*_sink).getScript();
}

}  // namespace recorder
}  // namespace gunit
