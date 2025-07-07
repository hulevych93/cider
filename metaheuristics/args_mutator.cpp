// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "metaheuristics/args_mutator.h"

#include <assert.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

namespace {
const std::string LOWERCASE = "abcdefghijklmnopqrstuvwxyz";
const std::string UPPERCASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string DIGITS = "0123456789";
const std::string SYMBOLS = "!@#$%^&*()_-+=<>?/{}~|";

// Aggregate all character sets
const std::string ALL_CHARS = LOWERCASE + UPPERCASE + DIGITS + SYMBOLS;

std::string generateRandomString(std::mt19937& gen, size_t length) {
  std::uniform_int_distribution<size_t> dist(0, ALL_CHARS.size() - 1);

  std::string randomStr;
  randomStr.reserve(length);

  for (size_t i = 0; i < length; ++i) {
    randomStr += ALL_CHARS[dist(gen)];
  }
  return randomStr;
}

bool mutateString(std::mt19937& gen, std::string& input) {
  std::uniform_int_distribution<size_t> charDist(0, ALL_CHARS.size() - 1);
  if (input.empty()) {
    input = generateRandomString(gen, charDist(gen));
  } else {
    std::uniform_int_distribution<size_t> indexDist(0, input.size() - 1);
    for (size_t i = 0; i < input.size() / 5; ++i) {
      input[indexDist(gen)] = ALL_CHARS[charDist(gen)];
    }
  }
  return true;
}

}  // namespace

namespace cider {
namespace metasearch {

std::ostream& operator<<(std::ostream& os, ArgsMutationStrategy strategy) {
  switch (strategy) {
    case ArgsMutationStrategy::ChangeBits:
      os << "ChangeBits";
      break;
    case ArgsMutationStrategy::ShuffleBytes:
      os << "ShuffleBytes";
      break;
    case ArgsMutationStrategy::LevyFlight:
      os << "LevyFlight";
      break;
    default:
      break;
  }
  return os;
}

namespace {

size_t randomInRange(std::mt19937& gen, const size_t from, const size_t to) {
  std::uniform_int_distribution<> distr(from, to);
  return distr(gen);
}  // namespace

struct ParamMutator final : cider::recorder::IParamMutator {
  ParamMutator(std::mt19937& gen,
               double rate,
               ArgsMutationStrategy strategy,
               bool mutateStrings)
      : _gen(gen),
        _mutationRate(rate),
        _strategy(strategy),
        _mtStr(mutateStrings) {}

  const ArgsMutationStrategy _strategy;
  double _mutationRate;
  std::mt19937& _gen;
  const bool _mtStr;

  std::optional<size_t> random(const size_t from,
                               const size_t to) const override {
    if (shouldMutate()) {
      return randomInRange(_gen, from, to);
    }
    return std::nullopt;
  }

  bool operator()(recorder::IntegerType& value) const {
    return std::visit([this](auto& val) { return (*this)(val); }, value);
  }

  bool operator()(recorder::Nil&) const override { return false; }

  bool operator()(bool& value) const override {
    if (shouldMutate()) {
      value = (bool)randomInRange(_gen, 0, 1);
      return true;
    }
    return false;
  }

  bool operator()(double& value) const override { return mutate<>(value); }
  bool operator()(float& value) const override { return mutate<>(value); }
  bool operator()(char*& value) const override { return false; }
  bool operator()(std::string& value) const override {
    if (_mtStr) {
      return mutateString(_gen, value);
    }
    return false;
  }
  bool operator()(std::wstring& /*value*/) const override { return false; }

  bool operator()(std::vector<std::string>& values) const override {
    bool result = false;
    if (_mtStr) {
      for (auto& elem : values) {
        result |= mutateString(_gen, elem);
      }
    }
    return result;
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
  bool mutate(Type& value) const {
    if (shouldMutate()) {
      switch (_strategy) {
        case ArgsMutationStrategy::None:
          return false;
          break;
        case ArgsMutationStrategy::ChangeBits:
          change_bit(value);
          return true;
          break;
        case ArgsMutationStrategy::ShuffleBytes:
          shaffle_bytes(value);
          return true;
          break;
        case ArgsMutationStrategy::LevyFlight:
          levy_flight(value);
          return true;
          break;
        default:
          break;
      }
    }
    return false;
  }

  bool shouldMutate() const {
    const bool shouldMutate =
        ((double)randomInRange(_gen, 0, 10000.f) / 10000.f) < _mutationRate;
    return shouldMutate;
  }

  template <typename Type>
  void shaffle_bytes(Type& value) const {
    auto* startData = reinterpret_cast<char*>(&value);
    const auto size = sizeof(Type);
    size_t ShuffleAmount = randomInRange(_gen, 0, sizeof(Type));
    size_t ShuffleStart = randomInRange(_gen, 0, sizeof(Type) - ShuffleAmount);
    assert(ShuffleStart + ShuffleAmount <= size);
    std::shuffle(startData + ShuffleStart,
                 startData + ShuffleStart + ShuffleAmount,
                 std::default_random_engine(std::time(0)));
  }

  template <typename Type>
  void change_bit(Type& value) const {
    auto* startData = reinterpret_cast<char*>(&value);
    size_t ShuffleAmount = randomInRange(_gen, 0, sizeof(Type) - 1);
    startData[ShuffleAmount] ^= 1 << randomInRange(_gen, 0, 7);
  }

  template <typename Type>
  void levy_flight(Type& value) const {
    std::cauchy_distribution<> levy_dist(0.0, 1.0);  // location=0, scale=1
    const double step = levy_dist(_gen) * 10.0;  // scale up for stronger jumps

    Type delta = static_cast<Type>(step);

    // Avoid zero mutation (do something at least)
    if (delta == 0) {
      delta = (std::uniform_int_distribution<>(0, 1)(_gen) == 0) ? 1 : -1;
    }

    // Apply mutation with bounds check
    if constexpr (std::is_signed_v<Type>) {
      value = std::clamp<Type>(value + delta, std::numeric_limits<Type>::min(),
                               std::numeric_limits<Type>::max());
    } else {
      if (delta < 0 && static_cast<std::uint64_t>(-delta) > value) {
        value = 0;
      } else {
        value += delta;
      }
    }
  }
};
}  // namespace

std::unique_ptr<recorder::IParamMutator> makeMutator(
    std::mt19937& gen,
    double mutationRate,
    ArgsMutationStrategy strategy,
    bool mutateStrings) {
  return std::make_unique<ParamMutator>(gen, mutationRate, strategy,
                                        mutateStrings);
}

}  // namespace metasearch
}  // namespace cider
