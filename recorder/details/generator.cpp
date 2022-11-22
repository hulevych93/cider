#include "generator.h"
#include "user_data.h"

#include <fmt/args.h>
#include <fmt/format.h>

namespace gunit {
namespace recorder {

namespace {

class CodeSinkImpl : public CodeSink {
 public:
  explicit CodeSinkImpl(std::string& sink) : _sink(sink) {}

  std::string processLocalVar(const char* varName, std::string&& code) override {
      fmt::dynamic_format_arg_store<fmt::format_context> fmt_args;
      fmt_args.push_back(fmt::arg(varName, varName));

      try {
          _sink += fmt::vformat(code, fmt_args);
      } catch (const fmt::format_error& fmtErr) {
          throw ScriptGenerationError{
              fmt::format(
                  "Local variable with name \'{}\' generation is failed. Details: \'{}\'",
                  varName, fmtErr.what())
                  .c_str()};
      }

    if (_sink.back() != '\n') {
      _sink += "\n";
    }

      return varName;
  }

 private:
  std::string& _sink;
};

struct ActionTemplateProducer final {
  std::string operator()(const FreeFunctionCall& functionCall) const {
    std::string funcTemplate =
        getFreeFunctionCallTemplate() + functionCall.functionName;
    const auto paramCount = functionCall.params.size();
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

void generate(const ParamCodeProducer paramProducer,
              std::string function,
              const Params& params,
              std::string& body) {
  std::vector<std::string> args;
  args.reserve(params.size());
  CodeSinkImpl sink(body);
  fmt::dynamic_format_arg_store<fmt::format_context> fmt_args;
  for (const auto& param : params) {
    args.emplace_back(paramProducer(param, sink));
    fmt_args.push_back(args.back());
  }

  try {
    body += fmt::vformat(function, fmt_args) + "\n";
  } catch (const fmt::format_error& fmtErr) {
    throw ScriptGenerationError{
        fmt::format(
            "Function with name \'{}\' generation is failed. Details: \'{}\'",
            function, fmtErr.what())
            .c_str()};
  }
}

}  // namespace

ScriptGenerationError::ScriptGenerationError(const char* msg)
    : _error(fmt::format("ScriptGenerationError: {}", msg)) {}

ScriptGenerator::ScriptGenerator(const ParamCodeProducer producer)
    : _paramProducer(producer) {}

void ScriptGenerator::operator()(const FreeFunctionCall& context) {
  generate(_paramProducer, produceActionTemplate(context), context.params,
           _body);
}

std::string ScriptGenerator::getScript() {
  return std::move(_body);
}

}  // namespace recorder
}  // namespace gunit
