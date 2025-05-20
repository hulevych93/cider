// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <variant>
#include <vector>

#include <memory>
#include <optional>
#include <sstream>
#include <string>

#include "utils/type_utils.h"

#include "recorder/details/sink.h"

#include "serialization/serializable.h"

namespace cider {
namespace recorder {

struct UserDataValueParam;
using UserDataValueParamPtr = std::shared_ptr<UserDataValueParam>;

struct UserDataReferenceParam;
using UserDataReferenceParamPtr = std::shared_ptr<UserDataReferenceParam>;

struct Nil final : serialization::SerializableTag {
  bool operator==(const Nil&) const { return true; }
};

inline bool serialize(const Nil&, serialization::Serializer&) {
  return true;
}
inline bool deserialize(Nil&, const serialization::Deserializer&) {
  return true;
}

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
    TypeList<Nil, bool, IntegerType, double, std::string, std::wstring>;

using Param = utils::ApplyTypeList<
    std::variant,
    utils::ConcatTypeLists<
        GeneratorTypesList,
        utils::ToTypeList<UserDataValueParamPtr, UserDataReferenceParamPtr>>>;
using Params = std::vector<Param>;

class CodeSink;

template <typename Type>
using PureType = std::remove_pointer_t<std::decay_t<Type>>;

class IParamMutator;

template <typename Type>
bool mutateAggregate(const IParamMutator& mutator, Type&);

struct IParamMutator {
  virtual ~IParamMutator() = default;

  virtual bool operator()(const char*&) const { return false; }

  virtual bool operator()(recorder::Nil&) const = 0;
  virtual bool operator()(bool& value) const = 0;
  virtual bool operator()(double& value) const = 0;
  virtual bool operator()(char*& value) const = 0;
  virtual bool operator()(std::string& value) const = 0;
  virtual bool operator()(std::wstring& value) const = 0;
  virtual bool operator()(std::vector<std::string>& value) const = 0;
  virtual bool operator()(float& value) const = 0;

  bool operator()(recorder::IntegerType& value) const {
    return std::visit([this](auto& val) { return (*this)(val); }, value);
  }

  virtual std::optional<size_t> random(const size_t from,
                                       const size_t to) const = 0;
  std::optional<size_t> random(const size_t max) const {
    return random(0, max);
  }

  virtual bool operator()(char& value) const = 0;
  virtual bool operator()(short& value) const = 0;
  virtual bool operator()(int& value) const = 0;
  virtual bool operator()(long& value) const = 0;
  virtual bool operator()(long long& value) const = 0;
  virtual bool operator()(unsigned char& value) const = 0;
  virtual bool operator()(unsigned short& value) const = 0;
  virtual bool operator()(unsigned int& value) const = 0;
  virtual bool operator()(unsigned long& value) const = 0;
  virtual bool operator()(unsigned long long& value) const = 0;

  template <
      typename EnumType,
      typename std::enable_if_t<std::is_enum_v<EnumType>, void*> = nullptr>
  bool operator()(EnumType& value) const {
    return mutateAggregate(*this, value);
  }

  virtual bool operator()(recorder::UserDataValueParamPtr& value) const = 0;
  virtual bool operator()(recorder::UserDataReferenceParamPtr& value) const = 0;
};

struct UserDataValueParam : public serialization::ISerializable {
 public:
  virtual ~UserDataValueParam() = default;
  virtual std::string generateCode(const std::string& moduleName,
                                   CodeSink& sink) const = 0;
  virtual bool mutate(const IParamMutator&) = 0;
  virtual void print(std::ostream& os) const { os << "obj@value"; }
  virtual UserDataValueParamPtr deepCopy() const = 0;
  virtual bool fuzzyEquals(const UserDataValueParam& param) const = 0;
  virtual size_t computeHash() const = 0;

  static std::shared_ptr<UserDataValueParam> Create(
      const serialization::Deserializer& deserializer);
};

struct UserDataReferenceParam : public serialization::ISerializable {
 public:
  virtual ~UserDataReferenceParam() = default;
  virtual std::string generateCode(const std::string& moduleName,
                                   CodeSink& sink) const = 0;
  virtual LocalVar registerLocal(CodeSink& sink) = 0;

  bool mutate(const IParamMutator&) { return false; }
  virtual void print(std::ostream& os) const { os << "obj@ref"; }
  virtual UserDataReferenceParamPtr deepCopy() const = 0;
  virtual bool fuzzyEquals(const UserDataReferenceParam& param) const = 0;
  virtual size_t computeHash() const = 0;

  static std::shared_ptr<UserDataReferenceParam> Create(
      const serialization::Deserializer& deserializer);
};

template <typename Type>
bool serializeAggregate(const Type&, serialization::Serializer&);

template <typename Type>
bool deserializeAggregate(Type&, const serialization::Deserializer&);

template <typename Type>
std::string produceAggregateCode(const std::string& moduleName,
                                 const Type&,
                                 CodeSink& sink);

template <typename Type>
bool mutateAggregate(const IParamMutator& mutator, Type&);

template <typename Type>
bool compareAggregates(const Type& lhs, const Type& rhs);

template <typename Type>
size_t hashAggregate(const Type&);

Param deepCopy(const Param& param);

std::ostream& print(std::ostream& os, const Param& param);

bool fuzzyEqual(const Param& lhs, const Param& rhs);

template <typename Type>
bool fuzzyEqual(const std::vector<Type>& lc, const std::vector<Type>& rc) {
  if (lc.size() != rc.size()) {
    return false;
  }
  auto fIt = lc.cbegin();
  auto sIt = rc.cbegin();
  for (; fIt < lc.cend(); ++fIt, ++sIt) {
    if (!fuzzyEqual(*fIt, *sIt)) {
      return false;
    }
  }
  return true;
}

template <typename Type>
std::vector<Type> deepCopy(const std::vector<Type>& container) {
  std::vector<Type> out;
  out.reserve(container.size());
  for (const auto& element : container) {
    out.push_back(deepCopy(element));
  }
  return out;
}

template <typename Type>
void print(std::ostream& os, const std::vector<Type>& container) {
  for (size_t i = 0; i < container.size(); ++i) {
    print(os, container[i]);
    if (i + 1 != container.size()) {
      os << ", ";
    }
  }
}

namespace details {

using CreateFn = std::function<std::shared_ptr<UserDataValueParam>()>;

inline std::unordered_map<size_t, CreateFn>&
createUserDataValueParamRegistry() {
  static std::unordered_map<size_t, CreateFn> instance;
  return instance;
}

template <typename Type>
struct AggregateUserDataValueParamImpl final : public UserDataValueParam {
 public:
  using ParamType = typename std::remove_pointer_t<std::decay_t<Type>>;
  static const bool ensureRegistered;

 public:
  AggregateUserDataValueParamImpl() = default;
  AggregateUserDataValueParamImpl(Type&& param)
      : _param(std::forward<Type>(param)) {}
  AggregateUserDataValueParamImpl(ParamType* param) : _param(*param) {}

  ~AggregateUserDataValueParamImpl() override = default;

  std::string generateCode(const std::string& moduleName,
                           CodeSink& sink) const override {
    return produceAggregateCode(moduleName, _param, sink);
  }

  bool mutate(const IParamMutator& mutator) override {
    return mutateAggregate(mutator, _param);
  }

  UserDataValueParamPtr deepCopy() const override {
    auto obj = std::make_shared<AggregateUserDataValueParamImpl<Type>>();
    obj->_param = _param;
    return obj;
  }

  bool serialize(serialization::Serializer& serializer) const override {
    const std::size_t key = type_key<Type>();
    serializer << key;
    return serializeAggregate(_param, serializer);
  }

  bool fuzzyEquals(const UserDataValueParam& param) const override {
    return compareAggregates(
        _param,
        static_cast<const AggregateUserDataValueParamImpl&>(param)._param);
  }

  virtual size_t computeHash() const override { return hashAggregate(_param); }

  bool deserialize(const serialization::Deserializer& deserializer) override {
    return deserializeAggregate(_param, deserializer);
  }

 private:
  ParamType _param;
};

template <typename Type>
struct AutoRegisterType {
  static bool Register() {
    const std::size_t key = type_key<Type>();
    createUserDataValueParamRegistry()[key] =
        []() -> std::shared_ptr<UserDataValueParam> {
      return std::make_shared<AggregateUserDataValueParamImpl<Type>>();
    };
    return true;
  }
};

template <typename T>
const bool AggregateUserDataValueParamImpl<T>::ensureRegistered =
    AutoRegisterType<T>::Register();

struct ReferenceUserDataValueParamImpl final : public UserDataReferenceParam {
 public:
  ReferenceUserDataValueParamImpl() = default;
  ReferenceUserDataValueParamImpl(const void* address)
      : _address((void*)address) {}
  ~ReferenceUserDataValueParamImpl() override = default;

  std::string generateCode(const std::string&, CodeSink& sink) const override {
    return sink.searchForLocalVar(_address);
  }

  LocalVar registerLocal(CodeSink& sink) override {
    return sink.registerLocalVar(_address);
  }

  bool fuzzyEquals(const UserDataReferenceParam&) const override {
    // Fow now all references are the same in our model of
    // state for qlearning approach.
    return true;
  }

  virtual size_t computeHash() const override { return 0xceaad3f; }

  void print(std::ostream& os) const override { os << "obj@" << _address; }

  UserDataReferenceParamPtr deepCopy() const override {
    return std::make_shared<ReferenceUserDataValueParamImpl>(_address);
  }

  bool serialize(serialization::Serializer& serializer) const override {
    serializer << _address;
    return true;
  }

  bool deserialize(const serialization::Deserializer& deserializer) override {
    deserializer >> _address;
    return true;
  }

 private:
  void* _address = nullptr;
};

template <typename Type>
constexpr bool isAggregate =
    std::is_aggregate_v<PureType<Type>> || std::is_enum_v<PureType<Type>>;

template <typename Type,
          typename std::enable_if_t<isAggregate<Type>, void*> = nullptr>
std::shared_ptr<UserDataValueParam> makeUserData(Type&& arg) {
  (void)AggregateUserDataValueParamImpl<Type>::ensureRegistered;
  return std::make_shared<AggregateUserDataValueParamImpl<Type>>(
      std::forward<Type>(arg));
}

template <typename Type,
          typename std::enable_if_t<isAggregate<Type>, void*> = nullptr>
std::shared_ptr<UserDataValueParam> makeUserData(Type* arg) {
  (void)AggregateUserDataValueParamImpl<Type>::ensureRegistered;
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
    std::is_constructible_v<std::string,
                            std::remove_reference_t<std::remove_cv_t<Type>>> &&
    !isUserData<Type>;

template <typename Type>
constexpr bool isStringVectorConvertibleType =
    !std::is_same_v<std::decay_t<Type>, std::vector<std::string>> &&
    std::is_constructible_v<std::vector<std::string>,
                            std::remove_reference_t<std::remove_cv_t<Type>>> &&
    !isUserData<Type>;

template <typename Type>
constexpr bool isWStringConvertibleType =
    !std::is_same_v<std::decay_t<Type>, std::string> &&
    std::is_constructible_v<std::wstring,
                            std::remove_reference_t<std::remove_cv_t<Type>>> &&
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

// inline Param makeParamImpl(const char* const argv[]) {
//   std::vector<std::string> vec;

//  if (argv == nullptr) {
//    return vec;
//  }

//  int argc = 0;
//  for (auto argvp = argv; *argvp; ++argc, ++argvp)
//    ;

//  vec.resize(static_cast<decltype(vec)::size_type>(argc));
//  std::transform(argv, argv + argc, vec.begin(),
//                 [](const char* const arg) { return arg; });

//  return vec;
//}

template <
    typename Type,
    typename std::enable_if_t<isWStringConvertibleType<Type>, void*> = nullptr>
Param makeParamImpl(Type arg) {
  return std::wstring{std::move(arg)};
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
  return Param(Nil{});
}

std::unique_ptr<IParamMutator> makeNullableMutator();

struct FuzzyParamHash final {
  template <typename T>
  void hash_combine(size_t& seed, const T& val) const {
    seed ^= (*this)(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

  size_t operator()(const cider::recorder::Param& param) const {
    return std::visit(
        [this](const auto& val) -> size_t {
          using T = std::decay_t<decltype(val)>;

          if constexpr (std::is_same_v<T, cider::recorder::Nil>) {
            return 0x9e3779b9;  // Fixed arbitrary value since Nil has no
                                // internal state
          } else if constexpr (std::is_same_v<T, bool> ||
                               std::is_same_v<T, double> ||
                               std::is_same_v<T, std::string> ||
                               std::is_same_v<T, std::wstring>) {
            return std::hash<T>{}(val);
          } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            size_t seed = 0x9b371cb9;
            for (const auto& el : val) {
              hash_combine(seed, el);
            }
            return seed;
          } else if constexpr (std::is_same_v<T,
                                              cider::recorder::IntegerType>) {
            return std::visit(
                [](auto&& integer) {
                  return std::hash<std::decay_t<decltype(integer)>>{}(integer);
                },
                val);
          } else if constexpr (
              std::is_same_v<T, cider::recorder::UserDataValueParamPtr> ||
              std::is_same_v<T, cider::recorder::UserDataReferenceParamPtr>) {
            return val->computeHash();
          } else {
            static_assert(!sizeof(T), "Unsupported type in Param");
          }
        },
        param);
  }

  size_t operator()(const cider::recorder::Params& params) const {
    size_t seed = 0;
    for (const auto& param : params) {
      hash_combine(seed, param);
    }
    return seed;
  }

  size_t operator()(const cider::recorder::Nil&) const noexcept {
    return 0x9e3779b9;  // Fixed arbitrary value since Nil has no internal state
  }
};

}  // namespace recorder
}  // namespace cider

namespace std {

template <typename T>
inline void hash_combine(size_t& seed, const T& val) {
  seed ^= hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}  // namespace std
