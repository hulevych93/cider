﻿#pragma once

#include <variant>
#include <vector>

#include <memory>
#include <optional>
#include <string>

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

using IntegerTypeList = TypeList<char,
                                 unsigned char,
                                 short,
                                 unsigned short,
                                 int,
                                 unsigned int,
                                 long,
                                 unsigned long,
                                 long long,
                                 unsigned long long>;

using IntegerType = utils::ApplyTypeList<std::variant, IntegerTypeList>;

using GeneratorTypesList =
    TypeList<Nil, bool, IntegerType, double, std::string>;

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
  virtual std::string generateCode(const std::string& moduleName,
                                   CodeSink& sink) const = 0;
};

struct UserDataReferenceParam : UserDataValueParam {
 public:
  virtual ~UserDataReferenceParam() = default;
  virtual LocalVar registerLocal(CodeSink& sink) = 0;
};

template <typename Type>
std::string produceAggregateCode(const std::string& moduleName,
                                 const Type&,
                                 CodeSink& sink);

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

  std::string generateCode(const std::string& moduleName,
                           CodeSink& sink) const override {
    return produceAggregateCode(moduleName, _param, sink);
  }

 private:
  ParamType _param;
};

struct ReferenceUserDataValueParamImpl final : public UserDataReferenceParam {
 public:
  ReferenceUserDataValueParamImpl(const void* address) : _address(address) {}
  ~ReferenceUserDataValueParamImpl() override = default;

  std::string generateCode(const std::string&, CodeSink& sink) const override {
    return sink.searchForLocalVar(_address);
  }

  LocalVar registerLocal(CodeSink& sink) override {
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
constexpr bool isIntegerType =
    utils::isTypeInTypeList<std::remove_reference_t<std::remove_cv_t<Type>>,
                            IntegerTypeList>();

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
    std::is_constructible_v<std::string, std::decay_t<Type>> &&
    !isUserData<Type>;

template <typename Type>
constexpr bool isFloatConvertibleType =
    !std::is_same_v<std::decay_t<Type>, double> &&
    std::is_floating_point_v<std::decay_t<Type>>;

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
  return static_cast<double>(arg);
}

template <typename Type,
          typename std::enable_if_t<isIntegerType<Type>, void*> = nullptr>
Param makeParamImpl(const Type arg) {
  return IntegerType{arg};
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
