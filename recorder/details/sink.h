#pragma once

#include <string>
#include <vector>

namespace cider {
namespace recorder {

class CodeSink {
 public:
  virtual ~CodeSink() = default;

  virtual std::string registerLocalVar(const void* object) = 0;

  virtual std::string searchForLocalVar(const void* object) const = 0;

  virtual std::string processLocalVar(const std::string& codeTemplate) = 0;

  virtual void processFunctionCall(
      const void* object,
      const std::string& resultName,
      const std::vector<std::string>& args,
      const std::string& codeTemplate) = 0;
};

}  // namespace recorder
}  // namespace cider
