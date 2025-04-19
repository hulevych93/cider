// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "metaheuristics/args_mutator.h"

#include <assert.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

namespace cider {
namespace metasearch {

namespace {

size_t randomInRange(std::mt19937& gen, const size_t from, const size_t to) {
  std::uniform_int_distribution<> distr(from, to);
  return distr(gen);
}  // namespace

struct ParamMutator final : cider::recorder::IParamMutator {
  ParamMutator(std::mt19937& gen, double rate, MutationStrategy strategy)
      : _gen(gen), _mutationRate(rate), _strategy(strategy) {}

  const MutationStrategy _strategy;
  double _mutationRate;
  std::mt19937& _gen;

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
  bool operator()(std::string& value) const override { return false; }
  bool operator()(std::wstring& value) const override { return false; }

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

  bool operator()(recorder::UserDataReferenceParamPtr& value) const override {
    return false;
  }

  template <typename Type>
  bool mutate(Type& value) const {
    if (shouldMutate()) {
      switch (_strategy) {
        case MutationStrategy::ChangeBits:
          change_bit(value);
          return true;
          break;
        case MutationStrategy::ShuffleBytes:
          shaffle_bytes(value);
          return true;
          break;
        default:
          break;
      }
    }
    return false;
  }

  bool shouldMutate() const {
    auto rnd = rand() / static_cast<double>(RAND_MAX);
    const bool shouldMutate = rnd < _mutationRate;
    if (shouldMutate) {
      // std::cout << "rnd: " << rnd << ", fl: " << shouldMutate << std::endl;
    }
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
};
}  // namespace

std::unique_ptr<recorder::IParamMutator>
makeMutator(std::mt19937& gen, double mutationRate, MutationStrategy strategy) {
  return std::make_unique<ParamMutator>(gen, mutationRate, strategy);
}

}  // namespace metasearch
}  // namespace cider
