// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <ostream>

namespace cider {
namespace agent_model {

struct RewardCounter final {
  int covGrow = 0;
  int trackGrow = 0;
  int covNearBest = 0;
  int twoSameAct = 0;
  int threeSameAct = 0;
  int sameCov = 0;
  int penalty = 0;
  int scriptLower = 0;
  int scriptSame = 0;
  int scriptBigger = 0;
};

void print(std::ostream& os, const RewardCounter& rwCounter);

}  // namespace agent_model
}  // namespace cider
