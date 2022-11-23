#pragma once

#include <string>
#include <vector>

namespace gunit {
namespace recorder {

class CodeSink {
 public:
  virtual ~CodeSink() = default;

  virtual std::string searchForLocalVar(const void* object) const = 0;

  //` Returns local var actual name, that was used in generated code.
  virtual std::string processLocalVar(const char* suggestedLocalName,
                                      const std::string& codeTemplate) = 0;

  //` Returns function result local var name, if returns any.
  //` Returns empty string otherwise.
  //` Could be constructor call, object method call or free-fuction call.
  virtual std::string processFunctionCall(const char* suggestedLocalName,
                                          const std::vector<std::string>& args,
                                          const std::string& codeTemplate) = 0;

  //` Returns function result local var name, if returns any.
  //` Returns empty string otherwise.
  //` Could be constructor call, object method call or free-fuction call.
  virtual std::string processConstructorCall(
      const void* object,
      const char* suggestedLocalName,
      const std::vector<std::string>& args,
      const std::string& codeTemplate) = 0;

  //` Returns function result local var name, if returns any.
  //` Returns empty string otherwise.
  //` Could be constructor call or free-fuction call.
  virtual std::string processMethodCall(const void* object,
                                        const char* suggestedLocalName,
                                        const std::vector<std::string>& args,
                                        const std::string& codeTemplate) = 0;
};

}  // namespace recorder
}  // namespace gunit
