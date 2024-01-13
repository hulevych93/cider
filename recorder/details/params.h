#pragma once

#include <variant>
#include <vector>

#include <memory>
#include <optional>
#include <string>

#include "utils/numeric_cast.h"
#include "utils/type_utils.h"

#include "recorder/details/sink.h"

namespace cider {
namespace recorder {

struct UserDataValueParam;
using UserDataValueParamPtr = std::shared_ptr<UserDataValueParam>;

struct UserDataReferenceParam;
using UserDataReferenceParamPtr = std::shared_ptr<UserDataReferenceParam>;

struct Nil final {
  bool operator==(const Nil&) const { return true; }
};

using GeneratorTypesList = TypeList<Nil, bool, int, float, std::string>;

using Param = utils::ApplyTypeList<
    std::variant,
    utils::ConcatTypeLists<
        GeneratorTypesList,
        utils::ToTypeList<UserDataValueParamPtr, UserDataReferenceParamPtr>>>;
using Params = std::vector<Param>;

class CodeSink;

struct UserDataValueParam {
 public:
  virtual ~UserDataValueParam() = default;
  virtual std::string generateCode(CodeSink& sink) const = 0;
};

struct UserDataReferenceParam : UserDataValueParam {
 public:
  virtual ~UserDataReferenceParam() = default;
  virtual std::string registerLocal(CodeSink& sink) = 0;
};

template <typename Type>
std::string produceAggregateCode(const Type&, CodeSink& sink);

namespace details {

template <typename Type>
struct AggregateUserDataValueParamImpl final : public UserDataValueParam {
 public:
  using ParamType = typename std::remove_pointer_t<std::decay_t<Type>>;

 public:
  AggregateUserDataValueParamImpl(Type&& param)
      : _param(std::forward<Type>(param)) {}
  AggregateUserDataValueParamImpl(ParamType* param) : _param(*param) {}

  ~AggregateUserDataValueParamImpl() override = default;

  std::string generateCode(CodeSink& sink) const override {
    return produceAggregateCode(_param, sink);
  }

 private:
  ParamType _param;
};

struct ReferenceUserDataValueParamImpl final : public UserDataReferenceParam {
 public:
  ReferenceUserDataValueParamImpl(const void* address) : _address(address) {}
  ~ReferenceUserDataValueParamImpl() override = default;

  std::string generateCode(CodeSink& sink) const override {
    return sink.searchForLocalVar(_address);
  }

  std::string registerLocal(CodeSink& sink) override {
    return sink.registerLocalVar(_address);
  }

 private:
  const void* _address;
};

template <typename Type>
using PureType = std::remove_pointer_t<std::decay_t<Type>>;

template <typename Type>
constexpr bool isAggregate =
    std::is_aggregate_v<PureType<Type>> || std::is_enum_v<PureType<Type>>;

template <typename Type,
          typename std::enable_if_t<isAggregate<Type>, void*> = nullptr>
std::shared_ptr<UserDataValueParam> makeUserData(Type&& arg) {
  return std::make_shared<AggregateUserDataValueParamImpl<Type>>(
      std::forward<Type>(arg));
}

template <typename Type,
          typename std::enable_if_t<isAggregate<Type>, void*> = nullptr>
std::shared_ptr<UserDataValueParam> makeUserData(Type* arg) {
  return std::make_shared<AggregateUserDataValueParamImpl<Type>>(arg);
}

template <typename Type,
          typename std::enable_if_t<!isAggregate<Type>, void*> = nullptr>
std::shared_ptr<UserDataReferenceParam> makeUserData(Type& arg) {
  return std::make_shared<ReferenceUserDataValueParamImpl>(std::addressof(arg));
}

template <typename Type,
          typename std::enable_if_t<!isAggregate<Type>, void*> = nullptr>
std::shared_ptr<UserDataReferenceParam> makeUserData(Type* arg) {
  return std::make_shared<ReferenceUserDataValueParamImpl>(arg);
}

template <typename Type>
constexpr bool isGeneratorType =
    utils::isTypeInTypeList<PureType<Type>, GeneratorTypesList>();

template <typename Type>
constexpr bool isUserData =
    !isGeneratorType<Type> &&
    (std::is_enum_v<PureType<Type>> || std::is_class_v<PureType<Type>>);

template <typename Type>
constexpr bool isStringConvertibleType =
    !std::is_same_v<std::decay_t<Type>, std::string> &&
    std::is_constructible_v<std::string, std::decay_t<Type>>;

template <typename Type>
constexpr bool isFloatConvertibleType =
    !std::is_same_v<std::decay_t<Type>, float> &&
    std::is_floating_point_v<std::decay_t<Type>>;

template <typename Type>
constexpr bool isIntConverbileType =
    !std::is_same_v<std::decay_t<Type>, int> &&
    !std::is_same_v<std::decay_t<Type>, bool> &&
    std::is_integral_v<std::decay_t<Type>>;

template <typename Type,
          typename std::enable_if_t<isUserData<Type>, void*> = nullptr>
Param makeParamImpl(Type&& arg) {
  return makeUserData(std::forward<Type>(arg));
}

template <
    typename Type,
    typename std::enable_if_t<isStringConvertibleType<Type>, void*> = nullptr>
Param makeParamImpl(Type arg) {
  return std::string{std::move(arg)};
}

template <
    typename Type,
    typename std::enable_if_t<isFloatConvertibleType<Type>, void*> = nullptr>
Param makeParamImpl(const Type arg) {
  return cider::numCast<float>(arg);
}

template <typename Type,
          typename std::enable_if_t<isIntConverbileType<Type>, void*> = nullptr>
Param makeParamImpl(const Type arg) {
  return cider::numCast<int>(arg);
}

template <typename Type,
          typename std::enable_if_t<isGeneratorType<Type>, void*> = nullptr>
Param makeParamImpl(Type&& arg) {
  return std::move(arg);
}

}  // namespace details

template <typename Type>
Param makeParam(Type&& arg) {
  return details::makeParamImpl(std::forward<Type>(arg));
}

template <typename Type>
Param makeParam(std::optional<Type> arg) {
  return arg ? details::makeParamImpl(std::move(arg.get())) : Nil{};
}

inline Param makeParam(std::nullopt_t) {
  return Nil{};
}

}  // namespace recorder
}  // namespace cider
