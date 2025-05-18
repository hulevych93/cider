// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "params.h"

#include <iostream>

namespace cider {
namespace recorder {

bool fuzzyEqual(const Param& lhs, const Param& rhs) {
  if (lhs.index() != rhs.index())
    return false;

  return std::visit(
      [](const auto& lhsVal, const auto& rhsVal) {
        using LhsType = std::decay_t<decltype(lhsVal)>;
        using RhsType = std::decay_t<decltype(rhsVal)>;
        if constexpr (std::is_same_v<LhsType, RhsType>) {
          if constexpr (std::is_same_v<LhsType, Nil> ||
                        std::is_same_v<LhsType, bool> ||
                        std::is_same_v<LhsType, double> ||
                        std::is_same_v<LhsType, std::string> ||
                        std::is_same_v<LhsType, std::wstring> ||
                        std::is_same_v<LhsType, IntegerType> ||
                        std::is_same_v<LhsType, std::vector<std::string>>) {
            return lhsVal == rhsVal;
          } else if constexpr (std::is_same_v<LhsType, UserDataValueParamPtr> ||
                               std::is_same_v<LhsType,
                                              UserDataReferenceParamPtr>) {
            return lhsVal->fuzzyEquals(*rhsVal);
          } else {
            static_assert(!sizeof(LhsType), "Unsupported type in Param");
          }

        } else {
          return false;
        }
      },
      lhs, rhs);
}

Param deepCopy(const Param& param) {
  return std::visit(
      [](auto&& value) -> Param {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, Nil> || std::is_same_v<T, bool> ||
                      std::is_same_v<T, double> ||
                      std::is_same_v<T, std::string> ||
                      std::is_same_v<T, std::wstring> ||
                      std::is_same_v<T, IntegerType> ||
                      std::is_same_v<T, std::vector<std::string>>) {
          return value;
        } else if constexpr (std::is_same_v<T, UserDataValueParamPtr> ||
                             std::is_same_v<T, UserDataReferenceParamPtr>) {
          return value->deepCopy();
        } else {
          static_assert(!sizeof(T), "Unsupported type in Param");
        }
      },
      param);
}

std::ostream& print(std::ostream& os, const cider::recorder::Param& param) {
  std::visit(
      [&os](auto&& value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, Nil>) {
          os << "Nil";
        } else if constexpr (std::is_same_v<T, bool> ||
                             std::is_same_v<T, double> ||
                             std::is_same_v<T, std::string>) {
          os << value;
        } else if constexpr (std::is_same_v<T, std::wstring>) {
          os << "Nan";  // TODO
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
          bool first = true;
          if (value.empty()) {
            os << "{}";
          }
          for (const auto& elem : value) {
            if (!first) {
              os << ',';
            }
            first = false;
            os << elem;
          }
        } else if constexpr (std::is_same_v<T, IntegerType>) {
          std::visit([&os](auto&& integer) { os << integer; }, value);
        } else if constexpr (std::is_same_v<T, UserDataValueParamPtr> ||
                             std::is_same_v<T, UserDataReferenceParamPtr>) {
          value->print(os);
        } else {
          static_assert(!sizeof(T), "Unsupported type in Param");
        }
      },
      param);
  return os;
}

std::shared_ptr<UserDataReferenceParam> UserDataReferenceParam::Create(
    const serialization::Deserializer&) {
  return std::make_shared<details::ReferenceUserDataValueParamImpl>();
}

std::shared_ptr<UserDataValueParam> UserDataValueParam::Create(
    const serialization::Deserializer& deserializer) {
  std::size_t key = 0;
  deserializer >> key;
  auto it = details::createUserDataValueParamRegistry().find(key);
  if (it != details::createUserDataValueParamRegistry().end()) {
    return it->second();
  }
  throw std::runtime_error{"Cant' deserialize UserDataValueParam"};
}

namespace details {

struct ParamNullableMutator final : cider::recorder::IParamMutator {
  std::optional<size_t> random(const size_t, const size_t) const override {
    return 0U;
  }

  bool operator()(recorder::IntegerType& value) const {
    return std::visit([this](auto& val) { return (*this)(val); }, value);
  }

  bool operator()(recorder::Nil&) const override { return false; }

  bool operator()(bool& value) const override {
    value = false;
    return false;
  }

  bool operator()(double& value) const override { return mutate<>(value); }
  bool operator()(float& value) const override { return mutate<>(value); }
  bool operator()(char*& value) const override { return false; }
  bool operator()(std::string& value) const override {
    value.clear();
    return true;
  }
  bool operator()(std::wstring& value) const override {
    value.clear();
    return true;
  }

  bool operator()(std::vector<std::string>& value) const override {
    value.clear();
    return true;
  }

  bool operator()(char& value) const override { return mutate<>(value); }
  bool operator()(short& value) const override { return mutate<>(value); }
  bool operator()(int& value) const override { return mutate<>(value); }
  bool operator()(long& value) const override { return mutate<>(value); }
  bool operator()(long long& value) const override { return mutate<>(value); }
  bool operator()(unsigned char& value) const override {
    return mutate<>(value);
  }
  bool operator()(unsigned short& value) const override {
    return mutate<>(value);
  }
  bool operator()(unsigned int& value) const override {
    return mutate<>(value);
  }
  bool operator()(unsigned long& value) const override {
    return mutate<>(value);
  }
  bool operator()(unsigned long long& value) const override {
    return mutate<>(value);
  }

  bool operator()(recorder::UserDataValueParamPtr& value) const override {
    return value->mutate(*this);
  }

  bool operator()(recorder::UserDataReferenceParamPtr&) const override {
    return false;
  }

  template <typename Type>
  static bool mutate(Type& value) {
    value = 0U;
    return true;
  }
};

}  // namespace details

std::unique_ptr<IParamMutator> makeNullableMutator() {
  return std::make_unique<details::ParamNullableMutator>();
}

}  // namespace recorder
}  // namespace cider
