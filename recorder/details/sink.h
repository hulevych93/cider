#pragma once

#include <string>
#include <vector>

namespace gunit {
namespace recorder {

class CodeSink {
 public:
  virtual ~CodeSink() = default;

  //` Registers given object as local var and returns it's name.
  virtual std::string registerLocalVar(const void* object) = 0;

  //` Returns local var name via object.
  virtual std::string searchForLocalVar(const void* object) const = 0;

  //` Returns local var actual name, that was used in generated code.
  virtual std::string processLocalVar(const std::string& codeTemplate) = 0;

  //` Returns function result local var name, if returns any.
  //` Returns empty string otherwise.
  //` Could be constructor call, object method call or free-fuction call.
  virtual void processFunctionCall(
      const void* object,
      const std::string& resultName,  // possibly results
      const std::vector<std::string>& args,
      const std::string& codeTemplate) = 0;
};

}  // namespace recorder
}  // namespace gunit
