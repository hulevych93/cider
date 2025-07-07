// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "synthesis/synthesis.h"

namespace cider {
namespace synthesis {

std::ostream& operator<<(std::ostream& os, const QSynthesisSettings& settings);

bool synthesize(std::mt19937& gen,
                const QSynthesisSettings& settings,
                ObjectiveFunction objFunc,
                const recorder::Actions& initial,
                recorder::Actions& out);

}  // namespace synthesis
}  // namespace cider
