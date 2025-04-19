// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"
#include "coverage/measurer.h"

namespace cider {
namespace metasearch {

enum class Algorithm { HarmonySearch, CuckooSearch };

using MeassureCallback = std::function<cider::coverage::ReportOpt(
    const std::vector<recorder::Action>&)>;

struct Solution final {
  std::vector<recorder::Action> actions;
  coverage::RootReport cov;
};

inline bool operator>(const Solution& left, const Solution& right) {
  return left.cov > right.cov;
}

inline bool operator<(const Solution& left, const Solution& right) {
  return left.cov < right.cov;
}

class IMetaSearch {
 public:
  virtual ~IMetaSearch() = default;

  virtual void initialize(const std::vector<recorder::Action>& actions) = 0;

  virtual void run() = 0;

  virtual const Solution& getBest() const = 0;
};

}  // namespace metasearch
}  // namespace cider
