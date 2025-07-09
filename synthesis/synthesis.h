// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/coverage.h"

#include <random>
#include <variant>

#include "test-case.h"

namespace cider {
namespace synthesis {

using ActionChoosing =
    std::function<std::optional<recorder::Action>(const TestScenario&)>;

enum class StopCondition { LimitActions, GreaterCoverage };

struct SynthesisSettingsBasic {
  const char* configName = "NAN";
  synthesis::StopCondition stopType = synthesis::StopCondition::GreaterCoverage;
  size_t limitActions = 450U;
  size_t maxRollback = 10U;
};

std::ostream& operator<<(std::ostream& os,
                         const SynthesisSettingsBasic& settings);

enum class GenerationStrategyType { Greedy, EGreedy, Boltzmann };

struct AgentSynthesisSettings : SynthesisSettingsBasic {
  GenerationStrategyType strategy = GenerationStrategyType::Greedy;
  float epsilon = 0.1f;
  float temperature = 1.0f;
};

std::ostream& operator<<(std::ostream& os,
                         const AgentSynthesisSettings& settings);

struct QSynthesisSettings final : AgentSynthesisSettings {};
struct SarsaSynthesisSettings final : AgentSynthesisSettings {};

struct RandSynthesisSettings final : SynthesisSettingsBasic {};

using SynthesisSettings = std::
    variant<RandSynthesisSettings, QSynthesisSettings, SarsaSynthesisSettings>;

namespace details {

bool synthesize(const SynthesisSettingsBasic& settings,
                const ActionChoosing& actionChoosing,
                TestScenario& testCase);

}  // namespace details

}  // namespace synthesis
}  // namespace cider
