// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "harmony.h"

namespace cider {
namespace metasearch {
namespace harmony_synthesis {

std::ostream& operator<<(std::ostream& os, const Settings& settings);

class Search final : public harmony::Search {
 public:
  explicit Search(const Settings& settings);

  harmony::Harmony generateHarmony(const harmony::Harmony& harmony) const;
  std::optional<harmony::Harmony> mutateHarmony(
      const harmony::Harmony& harmony) const;
};

}  // namespace harmony_synthesis
}  // namespace metasearch
}  // namespace cider
