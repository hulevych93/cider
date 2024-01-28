#pragma once

#include "recorder/details/params.h"

#include <memory>
#include <string>

namespace cider {
namespace recorder {

class CodeSink;

namespace lua {

struct ParamVisitor {
  template <
      typename Type,
      typename std::enable_if_t<details::isIntegerType<Type>, void*> = nullptr>
  std::string operator()(const Type arg) {
    return (*this)(IntegerType{arg});
  }

  std::string operator()(const Nil&) const;
  std::string operator()(const IntegerType& value) const;
  std::string operator()(bool value) const;
  std::string operator()(double value) const;
  std::string operator()(const char* value) const;
  std::string operator()(const std::string& value) const;
};

class UserDataParamVisitor final : public ParamVisitor {
 public:
  explicit UserDataParamVisitor(CodeSink& sink);

  using ParamVisitor::operator();

  std::string operator()(const UserDataValueParamPtr& value) const;

 private:
  CodeSink& _sink;
};

std::string produceParamCode(const Param& param, CodeSink& sink);

}  // namespace lua
}  // namespace recorder
}  // namespace cider
