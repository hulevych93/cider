// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "monte-carlo-tree-search/monte-carlo.h"

namespace cider {
namespace mcts {

void export_tree_dot_wrapper(const MCTSNode* root, std::ostream& os);

}  // namespace mcts
}  // namespace cider
