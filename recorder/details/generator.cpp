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

  std::string registerLocalVar(const void* object) override {
    return suggestLocalName(object);
  }

 private:
  std::string suggestLocalName(const void* object = nullptr) {
    if (object) {
      auto it = _locals.find(object);
      if (it == _locals.end()) {
        const auto suggested =
            std::string{"object"} + "_" + std::to_string(++_localCounter);
        _locals.emplace(object, suggested);
        return suggested;
      }
      return it->second;
    }

    return std::string{"object"} + "_" + std::to_string(++_localCounter);
  }

  std::string getObjectLocal(const void* object) const {
    const auto localIt = _locals.find(object);
    if (localIt != _locals.end()) {
      return localIt->second;
    }

    throw std::logic_error{"Object is unreachable."};
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

void ScriptGenerator::operator()(const Function& context) {
  const auto result = processResult(context.retVal);
  const auto codeTemplate =
      _langContext.funcProducer(_module.c_str(), context.name,
                                context.params.size(), !result.empty(), false);

  const auto args = produceArgs(context.params);
  _sink->processFunctionCall(nullptr, result, args, codeTemplate);
}

void ScriptGenerator::operator()(const ClassMethod& context) {
  const auto result = processResult(context.method.retVal);
  const auto codeTemplate = _langContext.funcProducer(
      _module.c_str(), context.method.name, context.method.params.size(),
      !result.empty(), true);

  const auto args = produceArgs(context.method.params);
  _sink->processFunctionCall(context.objectAddress, result, args, codeTemplate);
}

void ScriptGenerator::operator()(const ClassBinaryOp& context) {
  const auto codeTemplate = _langContext.binaryOpProducer(context.opName);

  const auto args = produceArgs({context.param});
  _sink->processFunctionCall(context.objectAddress, std::string{}, args,
                             codeTemplate);
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

std::string ScriptGenerator::processResult(const Param& param) {
  if (auto* userData = std::get_if<UserDataReferenceParamPtr>(&param)) {
    return (*userData)->registerLocal(*_sink);
  }
  return {};
}

std::string ScriptGenerator::getScript() {
  return static_cast<CodeSinkImpl&>(*_sink).getScript();
}

void ScriptGenerator::discard(){
    static_cast<CodeSinkImpl&>(*_sink).getScript();
}

}  // namespace recorder
}  // namespace gunit
