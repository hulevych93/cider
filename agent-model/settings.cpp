// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "settings.h"

namespace cider {
namespace agent_model {

std::ostream& operator<<(std::ostream& os,
                         const LearningSettingsBase& settings) {
  os << "conf[";
  os << settings.configName;
  os << "]_lr[";
  os << settings.learningRate;
  os << "]_df[";
  os << settings.discountFactor;
  os << "]_epds[";
  os << settings.episodes;
  os << "],_mxStDp[";
  os << settings.maxStateDepth;
  os << "]";
  return os;
}

}  // namespace agent_model
}  // namespace cider
