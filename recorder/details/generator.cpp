#include "generator.h"
#include "user_data.h"
#include "utils.h"

#include <fmt/args.h>
#include <fmt/format.h>

#include <sstream>

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

std::string ParamVisitor::operator()(const Nil&) const {
  return std::string{"nil"};
}

std::string ParamVisitor::operator()(const bool value) const {
  return value ? "true" : "false";
}

std::string ParamVisitor::operator()(const float value) const {
  // Independent from "C" locale conversion approach.
  // The decimal delimeter sign is always a dot.

  std::stringstream stream;
  stream.imbue(std::locale::classic());
  stream << value;
  return stream.str();
}

std::string ParamVisitor::operator()(const char* param) const {
  return (*this)(std::string{param});
}

std::string ParamVisitor::operator()(const std::string& value) const {
  return std::string{"'" + escape(value) + "'"};
}

UserDataParamVisitor::UserDataParamVisitor(CodeSink& sink) : _sink(sink) {}

std::string UserDataParamVisitor::operator()(
    const UserDataParamPtr& value) const {
  return (*value).generate(_sink);
}

ScriptGenerationError::ScriptGenerationError(const char* msg)
    : _error(fmt::format("ScriptGenerationError: {}", msg)) {}

namespace {

void generate(const char* function, const Params& params, std::string& body) {
  std::vector<std::string> args;
  args.reserve(params.size());
  CodeSinkImpl sink(body);
  fmt::dynamic_format_arg_store<fmt::format_context> fmt_args;
  for (const auto& param : params) {
    args.emplace_back(std::visit(UserDataParamVisitor{sink}, param));
    fmt_args.push_back(args.back());
  }

  assert(function);

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

void ScriptGenerator::operator()(const FreeFunctionCall& context) {
  generate(context.function, context.params, _body);
}

std::string ScriptGenerator::getScript() {
  std::string script;

  if (!_body.empty()) {
    script += std::move(_body);
  }

  return script;
}

}  // namespace recorder
}  // namespace gunit
