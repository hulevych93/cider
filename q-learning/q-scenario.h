// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"

#include "scenario.h"

#include "coverage/coverage.h"

#include "reward_counter.h"

namespace cider {
namespace qleaning {

class QScenario final : public Scenario {
 public:
  QScenario(RewardCounter& counter,
            std::mt19937& gen,
            int maxStateDepth,
            const recorder::Actions& initial,
            const synthesis::ObjectiveFunction& objFunc);

  std::optional<double> getReward() const;

 private:
  RewardCounter& _rwCounter;
};

}  // namespace qleaning
}  // namespace cider
