// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "harmony.h"

#include <assert.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

namespace cider {
namespace harmony {

namespace {

size_t randomInRange(const size_t from, const size_t to) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(from, to);
  return distr(gen);
}

struct ParamMutator final : cider::recorder::IParamMutator {
  ParamMutator(double rate, MutationStrategy strategy)
      : _mutationRate(rate), _strategy(strategy) {}

  const MutationStrategy _strategy;
  double _mutationRate;

  std::optional<size_t> random(const size_t from,
                               const size_t to) const override {
    if (shouldMutate()) {
      return randomInRange(from, to);
    }
    return std::nullopt;
  }

  void operator()(recorder::IntegerType& value) const {
    return std::visit([this](auto& val) { return (*this)(val); }, value);
  }

  void operator()(recorder::Nil&) const override {}
  void operator()(bool& value) const override {
    value = (bool)randomInRange(0, 1);
  }
  void operator()(double& value) const override { mutate<>(value); }
  void operator()(float& value) const override { mutate<>(value); }
  void operator()(char*& value) const override {}
  void operator()(std::string& value) const override {}

  void operator()(char& value) const override { mutate<>(value); }
  void operator()(short& value) const override { mutate<>(value); }
  void operator()(int& value) const override { mutate<>(value); }
  void operator()(long& value) const override { mutate<>(value); }
  void operator()(long long& value) const override { mutate<>(value); }
  void operator()(unsigned char& value) const override { mutate<>(value); }
  void operator()(unsigned short& value) const override { mutate<>(value); }
  void operator()(unsigned int& value) const override { mutate<>(value); }
  void operator()(unsigned long& value) const override { mutate<>(value); }
  void operator()(unsigned long long& value) const override { mutate<>(value); }

  void operator()(recorder::UserDataValueParamPtr& value) const override {
    value->mutate(*this);
  }

  void operator()(recorder::UserDataReferenceParamPtr& value) const override {}

  template <typename Type>
  void mutate(Type& value) const {
    if (shouldMutate()) {
      switch (_strategy) {
        case MutationStrategy::ChangeBits:
          change_bit(value);
          break;
        case MutationStrategy::ShuffleBytes:
          shaffle_bytes(value);
          break;
        default:
          break;
      }
    }
  }

  bool shouldMutate() const {
    return rand() / static_cast<double>(RAND_MAX) < _mutationRate;
  }

  template <typename Type>
  void shaffle_bytes(Type& value) const {
    auto* startData = reinterpret_cast<char*>(&value);
    const auto size = sizeof(Type);
    size_t ShuffleAmount = randomInRange(0, sizeof(Type));
    size_t ShuffleStart = randomInRange(0, sizeof(Type) - ShuffleAmount);
    assert(ShuffleStart + ShuffleAmount <= size);
    std::shuffle(startData + ShuffleStart,
                 startData + ShuffleStart + ShuffleAmount,
                 std::default_random_engine(std::time(0)));
  }

  template <typename Type>
  void change_bit(Type& value) const {
    auto* startData = reinterpret_cast<char*>(&value);
    size_t ShuffleAmount = randomInRange(0, sizeof(Type) - 1);
    startData[ShuffleAmount] ^= 1 << randomInRange(0, 7);
  }
};
}  // namespace

std::unique_ptr<recorder::IParamMutator> makeMutator(
    double mutationRate,
    MutationStrategy strategy) {
  return std::make_unique<ParamMutator>(mutationRate, strategy);
}

}  // namespace harmony
}  // namespace cider
