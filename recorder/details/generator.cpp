#include "generator.h"
#include "user_data.h"

#include <fmt/args.h>
#include <fmt/format.h>

namespace gunit {
namespace recorder {

namespace {
using ParamsListConstIt = std::vector<Params>::const_iterator;

class CodeSinkImpl : public CodeSink {
 public:
  explicit CodeSinkImpl(std::string& sink) : _sink(sink) {}

  void addLocal(const char*, std::string&& code) override {
    _sink += std::move(code);

    if (_sink.back() != '\n') {
      _sink += "\n";
    }
  }

 private:
  std::string& _sink;
};

}  // namespace

void ScriptGenerator::operator()(const FreeFunctionCall& context) {
  _body += generate(context.function, context.params);
}

std::string ScriptGenerator::getScript() {
  std::string script;

  if (!_body.empty()) {
    script += std::move(_body);
  }

  return script;
}

std::string ScriptGenerator::generate(const char* function,
                                      const Params& params) {
  std::vector<Argument> args;
  args.reserve(params.size());
  CodeSinkImpl sink(_body);
  fmt::dynamic_format_arg_store<fmt::format_context> fmt_args;
  for (const auto& param : params) {
    args.emplace_back(std::visit(UserDataParamVisitor{sink}, param));
    fmt_args.push_back(args.back());
  }

  assert(function);

  try {
    return fmt::vformat(function, fmt_args) + "\n";
  } catch (const fmt::format_error& fmtErr) {
    throw ScriptGenerationError{
        fmt::format(
            "Function with name \'{}\' generation is failed. Details: \'{}\'",
            function, fmtErr.what())
            .c_str()};
  }
}

}  // namespace recorder
}  // namespace gunit
