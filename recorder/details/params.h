#pragma once

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
  bool operator==(const Nil&) const { return true; }
};

//` The `GeneratorTypesList` includes all the fundamental types.
using GeneratorTypesList = TypeList<Nil, bool, int, float, std::string>;

//` User action parameters are introduced by the `Param` variant.
using Param = utils::ApplyTypeList<
    std::variant,
    utils::ConcatTypeLists<GeneratorTypesList,
                           utils::ToTypeList<UserDataParamPtr>>>;
using Params = std::vector<Param>;

}  // namespace recorder
}  // namespace gunit
