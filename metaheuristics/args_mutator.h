// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include <random>

namespace cider {
namespace metasearch {

enum class MutationStrategy { ShuffleBytes, ChangeBits };

std::unique_ptr<recorder::IParamMutator> makeMutator(std::mt19937& gen,
                                                     double mutationRate,
                                                     MutationStrategy strategy);

}  // namespace metasearch
}  // namespace cider
