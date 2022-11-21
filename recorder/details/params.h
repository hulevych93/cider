#pragma once

#include <optional>
#include <variant>
#include <vector>

#include <memory>
#include <string>

#include "utils/type_utils.h"

namespace gunit {
namespace recorder {

//` The `UserDataParam` is any user defined type to be passed as parameter to
// user action.
struct UserDataParam;
using UserDataParamPtr = std::shared_ptr<UserDataParam>;

//` The `Nil` is the tag type for lua 'nil' param generation.
struct Nil final {
  //` Every two `Nil` objects are always equal.
  bool operator==(const Nil&) const;
};

namespace details {
using FundumentalTypesList = TypeList<bool, int, float, std::string>;
using GenerationTagsTypesList = TypeList<Nil>;
}  // namespace details

//` The `GeneratorTypesList` includes all the fundamental types + pre-defined
// generation tags.
using GeneratorTypesList =
    utils::ConcatTypeLists<details::FundumentalTypesList,
                           details::GenerationTagsTypesList>;

//` User action parameters are introduced by the `Param` variant.
using Param = utils::ApplyTypeList<
    std::variant,
    utils::ConcatTypeLists<GeneratorTypesList,
                           utils::ToTypeList<UserDataParamPtr>>>;
using Params = std::vector<Param>;

//` The `Argument` to be passed into the function call.
using Argument = std::string;

//` The `ParamVisitor` produces code chunks to be used as arguments during
//` code generation.
struct ParamVisitor {
  template <
      typename Type,
      std::enable_if_t<std::is_integral_v<Type> &&
                       !std::is_same_v<std::decay_t<Type>, bool>>* = nullptr>
  Argument operator()(Type param) {
    return std::to_string(param);
  }

  Argument operator()(const Nil&) const;
  Argument operator()(bool value) const;
  Argument operator()(float value) const;
  Argument operator()(const char* value) const;
  Argument operator()(const std::string& value) const;
};

class CodeSink;

//` The `UserDataParamVisitor` is extended with UserDataParamPtr operator
// overload.
class UserDataParamVisitor final : public ParamVisitor {
 public:
  explicit UserDataParamVisitor(CodeSink& sink);

  using ParamVisitor::operator();

  Argument operator()(const UserDataParamPtr& value) const;

 private:
  CodeSink& _sink;
};

}  // namespace recorder
}  // namespace gunit
